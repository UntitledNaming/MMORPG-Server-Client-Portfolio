

#include "M1SpawnManager.h"
#include "Network\M1NetworkManager.h"
#include "ContentsDefine.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Controller\M1PlayerController.h"
#include "Character\M1Character.h"
#include "Character\M1BasePlayer.h"
#include "Character\M1LocalPlayer.h"
#include "Character\M1OtherPlayer.h"
#include "Character\M1Monster.h"
#include "Ability\M1PlayerActorComponent.h"
#include "Network\ClientCore\CMessage.h"
#include "NetPacketHeader.h"
#include "Kismet/GameplayStatics.h"

AM1SpawnManager::AM1SpawnManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
}

void AM1SpawnManager::BeginPlay()
{
    Super::BeginPlay();

    UGameInstance* GI = GetGameInstance();
    if (GI == nullptr)
        return;


    NetworkManager = GI->GetSubsystem<UM1NetworkManager>();
    if (NetworkManager == nullptr)
        return;

    NetworkManager->SetSpawnManager(this);

    OldRTTCheckTime = FPlatformTime::Seconds();
}

void AM1SpawnManager::Tick(float Deltatime)
{
    Super::Tick(Deltatime);

    TickClock(Deltatime);

    double curTime = FPlatformTime::Seconds();
    if (curTime - OldRTTCheckTime >= 1.0f)
    {
        SendRttPacket();
        OldRTTCheckTime = curTime;
    }
}

void AM1SpawnManager::SpawnMyPlayer(FM1SpawnData& Data)
{
    if (LocalPlayerCharacterClass == nullptr)
        return;


    // 액터 생성은 월드가 있어야 가능하니 월드 얻는데 없으면 그냥 pass
    UWorld* World = GetWorld();
    if (World == nullptr)
        return;

    // 어쨌든 생성하겠다는 의미. 겹치면 위치 조정해서
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AM1LocalPlayer* NewPlayer =
        World->SpawnActor<AM1LocalPlayer>(LocalPlayerCharacterClass, Data.Location, Data.Rotation, Params);

    if (NewPlayer == nullptr)
        return;

    // 플레이어 생성하면 플래그 키고 플레이어 초기화 후 PlayerMap에 넣기
    MyID = Data.EntityID;
    Data.CurrentEXP = 50.f;
    Data.RequiredEXP = 100.f;
    Data.Level = 1;
    NewPlayer->ApplySpawnData(Data);
    NewPlayer->SetSpawnFlag(true);
    MyPlayer = NewPlayer;


    // 컨트롤러 가져와서 이 캐릭터에 Possess함. 컨트롤러가 지금 생성한 캐릭터 컨트롤 할 수 있게 함.
    // 이 액터를 소유한 로컬 플레이어 찾기
    APlayerController* LocalPC = nullptr;
    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* TestPC = It->Get();
        if (TestPC && TestPC->IsLocalController())
        {
            LocalPC = TestPC;
            break;
        }
    }

    AM1PlayerController* PC = Cast<AM1PlayerController>(LocalPC);
    if (PC)
    {
        PC->SetCachedPlayer(NewPlayer);
        PC->OnPossess(NewPlayer);
        PC->SetControlRotation(FRotator(0.f, 0.f, 0.f));
        NewPlayer->SetActorRotation(FRotator(0.f, 0.f, 0.f));
        
        SendRttPacket();
    }

}

void AM1SpawnManager::SpawnOtehrPlayer(FM1SpawnData& Data)
{
    if (PlayerMap.Contains(Data.EntityID))
        return;

    if (OtherPlayerCharacterClass == nullptr)
        return;

    UWorld* World = GetWorld();
    if (World == nullptr)
        return;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AM1OtherPlayer* NewCharacter =
        World->SpawnActor<AM1OtherPlayer>(
            OtherPlayerCharacterClass,
            Data.Location,
            Data.Rotation,
            Params);

    if (NewCharacter == nullptr)
        return;

    NewCharacter->ApplySpawnData(Data);
    NewCharacter->SetSpawnFlag(true);

    FString Name = TEXT("GreyStone");
    NewCharacter->InitOverheadStatus(Name, Data.HP, Data.MaxHP);
    NewCharacter->SetOverheadVisible(true);

    PlayerMap.Add(Data.EntityID, NewCharacter);
}

void AM1SpawnManager::SpawnMonster(FM1SpawnData& Data)
{

}

void AM1SpawnManager::DespawnPlayer(uint64 EntityID)
{
    AM1OtherPlayer** Found = PlayerMap.Find(EntityID);
	if (Found == nullptr || *Found == nullptr)
		return;

	(*Found)->Destroy();
	PlayerMap.Remove(EntityID);
}

void AM1SpawnManager::DespawnMonster(uint64 EntityID)
{
    AM1Monster** Found = MonsterMap.Find(EntityID);
    if (Found == nullptr || *Found == nullptr)
        return;

    (*Found)->Destroy();
    MonsterMap.Remove(EntityID);
}

void AM1SpawnManager::SyncMyPlayer(FVector& Location)
{
    if (LocalPlayerCharacterClass == nullptr || MyPlayer == nullptr)
        return;

    MyPlayer->SetActorLocation(Location);
}

void AM1SpawnManager::SyncOtherPlayer(uint64 EntityID, FVector& Location, uint64 ServerTimestamp)
{
    AM1OtherPlayer** Found = PlayerMap.Find(EntityID);
    if (Found == nullptr || *Found == nullptr)
        return;

    (*Found)->OnReceiveSyncPacket(ServerTimestamp, Location);
}

void AM1SpawnManager::UpdateOtherPlayerMovementInput(uint64 EntityID, FMovementSnapshot& Snapshot)
{
    AM1OtherPlayer** Found = PlayerMap.Find(EntityID);
    if (Found == nullptr || *Found == nullptr)
        return;

    (*Found)->OnReceiveMovementPacket(Snapshot);
}

void AM1SpawnManager::OnOtherPlayerAttackSwing(uint64 EntityID, float FacingYaw, uint8 SwingIdx)
{
    AM1OtherPlayer** Found = PlayerMap.Find(EntityID);
    if (Found == nullptr || *Found == nullptr)
        return;

    (*Found)->OnReceiveAttackSwing(FacingYaw, SwingIdx);
}

void AM1SpawnManager::OnOtherPlayerAttackStop(uint64 EntityID)
{
    AM1OtherPlayer** Found = PlayerMap.Find(EntityID);
    if (Found == nullptr || *Found == nullptr)
        return;

    (*Found)->OnReceiveAttackStop();
}

void AM1SpawnManager::ProcessClientAttackHit(FVector Origin, FVector Forward, float Range, float HalfAngleDeg)
{
    float CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(HalfAngleDeg));
    float RangeSquared = Range * Range;

    // PlayerMap 순회
    for (TMap<uint64, AM1OtherPlayer*>::TIterator It(PlayerMap); It; ++It)
    {
        AM1Character* Target = It.Value();
        if (!Target)
            continue;

        // 공격 캐릭터와 탐색한 피격 대상 캐릭터간 위치벡터를 ToTarget에 저장해서 거리가 Range 벗어나면 피격 대상아님
        FVector ToTarget = Target->GetActorLocation() - Origin;
        if (ToTarget.SizeSquared() > RangeSquared)
            continue;

        // 위치벡터 정규화 
        ToTarget.Normalize();

        // 공격자의 앞방향 벡터와 타겟과의 방향벡터를 내적해서 공격자 앞방향 벡터와 방향벡터 사이의 사잇각에 대한 Cos를 구함.
        // 그게 공격범위각도의 절반값에 대해서 Cos값을 취한 CosHalfAngle 보다 작으면 공격 각도 범위안에 있다는 말임.
        if (FVector::DotProduct(Forward, ToTarget) < CosHalfAngle)
            continue;

        // 피격자에서 공격자로 가는 위치 벡터 계산
        FVector ToAttacker = (Origin - Target->GetActorLocation()).GetSafeNormal2D();

        // 피격자가 보는 앞방향 벡터
        FVector TargetFwd = Target->GetActorForwardVector().GetSafeNormal2D();

        // 피격자의 우측 방향 벡터
        FVector TargetRight = Target->GetActorRightVector().GetSafeNormal2D();


        // 피격 당한 캐릭터 기준으로 공격자가 어느 방향에 있는지 각도 계산
        // 0° = 정면, +90° = 우측, -90° = 좌측, ±180° = 후면
        // 공격자가 피격자의 앞에서 공격하면 HitAngle +0도
        float HitAngle = FMath::RadiansToDegrees(
            FMath::Atan2(
                FVector::DotProduct(TargetRight, ToAttacker),
                FVector::DotProduct(TargetFwd, ToAttacker)
            ));

        // 타겟에게 피격각도 전달해서 각도에 따른 피격 애니재생하기
        Target->TriggerHitReact(HitAngle);
    }

    // MonsterMap 순회
    for (TMap<uint64, AM1Monster*>::TIterator It(MonsterMap); It; ++It)
    {
        AM1Character* Target = It.Value();
        if (!Target)
            continue;

        FVector ToTarget = Target->GetActorLocation() - Origin;
        if (ToTarget.SizeSquared() > RangeSquared)
            continue;

        ToTarget.Normalize();
        if (FVector::DotProduct(Forward, ToTarget) < CosHalfAngle)
            continue;

        // 피격 당한 캐릭터 기준으로 공격자가 어느 방향에 있는지 각도 계산
        FVector ToAttacker = (Origin - Target->GetActorLocation()).GetSafeNormal2D();
        FVector TargetFwd = Target->GetActorForwardVector().GetSafeNormal2D();
        FVector TargetRight = Target->GetActorRightVector().GetSafeNormal2D();

        float HitAngle = FMath::RadiansToDegrees(
            FMath::Atan2(
                FVector::DotProduct(TargetRight, ToAttacker),
                FVector::DotProduct(TargetFwd, ToAttacker)
            ));

        Target->TriggerHitReact(HitAngle);
    }
}

void AM1SpawnManager::ApplyPlayerHitResult(uint64 EntityID, int32 NewHP)
{
    if (AM1Character* Character = FindPlayer(EntityID))
    {
        Character->SetHP(NewHP);
        return;
    }
}

void AM1SpawnManager::ApplyMonsterHitResult(uint64 EntityID, int32 NewHP)
{
    if (AM1Character* Monster = FindMonster(EntityID))
    {
        Monster->SetHP(NewHP);
        return;
    }
}

AM1Character* AM1SpawnManager::FindPlayer(uint64 EntityID) const
{
    AM1OtherPlayer* const* Found = PlayerMap.Find(EntityID);

	if (Found == nullptr)
		return nullptr;

	return *Found;
}

AM1Character* AM1SpawnManager::FindMonster(uint64 EntityID) const
{
    AM1Monster* const* Found = MonsterMap.Find(EntityID);

    if (Found == nullptr)
        return nullptr;

    return *Found;
}

void AM1SpawnManager::SendRttPacket()
{
    if (NetworkManager == nullptr)
        return;

    CMessage* pMessage = CMessage::Alloc();
    pMessage->Clear(1);

    PendingPingSentTime = GetLocalTimeMs();
    mpCreateRttPacket(pMessage, PendingPingSentTime);

    NetworkManager->SendPacket(pMessage, static_cast<uint8>(ERouteType::GROUP), ServiceID::NONE_SERVICE);

    CMessage::Free(pMessage);
}

void AM1SpawnManager::mpCreateRttPacket(CMessage* pMessage, double Time)
{
    *pMessage << FieldProtocol::PACKET_CS_RTT_SEND;
    *pMessage << Time;
}

void AM1SpawnManager::GetRTTEchoMsg(uint64 ServerStampMs)
{
    const uint64 RecvTime = GetLocalTimeMs();

    if (PendingPingSentTime == 0 || RecvTime < PendingPingSentTime)
        return;

    const uint64 RTT_ms = RecvTime - PendingPingSentTime;

    // 비정상적으로 늦은 RTT 응답 패킷은 RTT 기록안하고 버림.
    constexpr uint64 MAX_VALID_RTT_MS = 1000;
    if (RTT_ms > MAX_VALID_RTT_MS)
    {
        PendingPingSentTime = 0;
        return;
    }

    const uint64 HalfRTT = RTT_ms / 2;
    const uint64 EstimatedServerNow = ServerStampMs + HalfRTT;     // 받은 그 순간 추정 서버 시간
    const int64  NewOffset = (int64)EstimatedServerNow - (int64)RecvTime;

    // 최근 N개 샘플 보관
    OffsetSamples.Add({ NewOffset, RTT_ms });
    constexpr int32 MAX_SAMPLES = 8;
    if (OffsetSamples.Num() > MAX_SAMPLES) OffsetSamples.RemoveAt(0);

    // RTT 가장 작은 샘플의 offset을 target으로. RTT 작을수록 RTT/2 가정 신뢰도 높음.
    FOffsetSample Best = OffsetSamples[0];
    for (const FOffsetSample& S : OffsetSamples)
        if (S.RTT_ms < Best.RTT_ms) Best = S;


    TargetOffsetMs = Best.Offset;

    // 첫 sync일 때만 즉시 적용 (이후엔 TickClock에서 점진 보정)
    if (!bSynced)
    {
        ClockOffsetMs = TargetOffsetMs;
        bSynced = true;
    }

    PendingPingSentTime = 0;

    GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green,
        FString::Printf(TEXT("RTT : %lld"), RTT_ms));

    GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green,
        FString::Printf(TEXT("ClockOffsetMs : %lld"), ClockOffsetMs));

    GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green,
        FString::Printf(TEXT("TargetOffsetMs : %lld"), TargetOffsetMs));

}

void AM1SpawnManager::TickClock(float DeltaTime)
{
    if (!bSynced) return;

    const int64 Diff = TargetOffsetMs - ClockOffsetMs;
    if (Diff == 0) return;

    // 초당 50ms 속도로 따라감. 너무 빠르면 보간 캐릭터 위치가 출렁임.
    const int64 MaxStepMs = FMath::Max((int64)1, (int64)(25.0 * DeltaTime));
    if (FMath::Abs(Diff) <= MaxStepMs)
        ClockOffsetMs = TargetOffsetMs;
    else
        ClockOffsetMs += (Diff > 0) ? MaxStepMs : -MaxStepMs;
}

void AM1SpawnManager::ProcessClientSingleTargetHit(FVector Origin, FVector Forward, float Range, float HalfAngleDeg)
{
    float CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(HalfAngleDeg));
    float RangeSquared = Range * Range;

    AM1Character* BestTarget  = nullptr;
    float         BestDistSq  = TNumericLimits<float>::Max();

    auto CheckTarget = [&](AM1Character* Target)
    {
        if (!Target) return;
        FVector ToTarget = Target->GetActorLocation() - Origin;
        float   DistSq   = ToTarget.SizeSquared();
        if (DistSq > RangeSquared) return;
        ToTarget.Normalize();
        if (FVector::DotProduct(Forward, ToTarget) < CosHalfAngle) return;
        if (DistSq < BestDistSq) { BestDistSq = DistSq; BestTarget = Target; }
    };

    for (auto& Pair : PlayerMap)  CheckTarget(Pair.Value);
    for (auto& Pair : MonsterMap) CheckTarget(Pair.Value);

    if (!BestTarget) return;

    FVector ToAttacker  = (Origin - BestTarget->GetActorLocation()).GetSafeNormal2D();
    FVector TargetFwd   = BestTarget->GetActorForwardVector().GetSafeNormal2D();
    FVector TargetRight = BestTarget->GetActorRightVector().GetSafeNormal2D();
    float   HitAngle    = FMath::RadiansToDegrees(
        FMath::Atan2(FVector::DotProduct(TargetRight, ToAttacker),
                     FVector::DotProduct(TargetFwd,   ToAttacker)));

    BestTarget->TriggerHitReact(HitAngle);
}

void AM1SpawnManager::OnMyPlayerSkillResponse(EAbilitySlot Slot, bool bSuccess)
{
    if (!MyPlayer) return;
    if (UM1PlayerActorComponent* AC = MyPlayer->GetAbilityComponent())
        AC->OnSkillResponse(Slot, bSuccess);
}

void AM1SpawnManager::OnOtherCharacterUseSkill(uint64 EntityID, EAbilitySlot Slot)
{
    AM1OtherPlayer** Found = PlayerMap.Find(EntityID);
    if (Found && *Found)
    {
        if (UM1PlayerActorComponent* AC = (*Found)->GetAbilityComponent())
            AC->TriggerAbilityFX(Slot);
    }
}
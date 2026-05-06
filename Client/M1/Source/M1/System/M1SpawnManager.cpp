

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
        PC->Possess(NewPlayer);
        PC->SetCachedPlayer(NewPlayer);
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
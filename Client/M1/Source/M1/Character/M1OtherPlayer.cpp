#include "Character/M1OtherPlayer.h"
#include "Network\M1NetworkManager.h"
#include "System\M1SpawnManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "System\M1GameInstance.h"
#include "ContentsDefine.h"

AM1OtherPlayer::AM1OtherPlayer()
{
	PrimaryActorTick.bCanEverTick = true;
}

float AM1OtherPlayer::GetMoveSpeed()
{
	return GetCharacterMovement()->MaxWalkSpeed;
}

bool  AM1OtherPlayer::GetMoveFlag()
{
    if (bNeedStopCorrection)
        return true;

	return bMoving;
}

void  AM1OtherPlayer::SetHP(int32 NewHP)
{
    Super::SetHP(NewHP);

    SetOverheadHP(NewHP, MaxHP);
}

void AM1OtherPlayer::BeginPlay()
{
    Super::BeginPlay();

    NetworkManager = GetGameInstance()->GetSubsystem<UM1NetworkManager>();
}

void AM1OtherPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateInterpolation(DeltaTime);
	UpdateMoveDirection();

	if (bNeedStopCorrection)
		UpdateStopCorrection(DeltaTime);


	DbgLogTimer += DeltaTime;
	if (DbgLogTimer >= 2.f)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				(int32)EntityID,
				2.5f,
				FColor::Yellow,
				FString::Printf(
					TEXT("[OP %llu] TooFew=%d NoRTT=%d Early=%d ExhStop=%d Extrap=%d Idle=%d NoPair=%d StopIdle=%d | OK=%d Buf=%d"),
					EntityID,
					DbgCount_TooFewSnaps, DbgCount_NoRTT, DbgCount_TooEarly,
					DbgCount_ExhStop, DbgCount_ExhExtrap, DbgCount_ExhIdle,
					DbgCount_NoPair, DbgCount_StopNoMove,
					DbgCount_Rendered, SnapshotBuffer.Count));
		}
		DbgLogTimer          = 0.f;
		DbgCount_TooFewSnaps = DbgCount_NoRTT      = DbgCount_TooEarly   = 0;
		DbgCount_ExhStop     = DbgCount_ExhExtrap  = DbgCount_ExhIdle    = 0;
		DbgCount_NoPair      = DbgCount_StopNoMove = DbgCount_Rendered   = 0;
	}
}

void AM1OtherPlayer::OnReceiveMovementPacket(const FMovementSnapshot& Snapshot)
{
	SnapshotBuffer.Add(Snapshot);
}

void AM1OtherPlayer::UpdateMoveDirection()
{
    FVector Delta = GetActorLocation() - PrevLocation;
    PrevLocation  = GetActorLocation();

    if (Delta.SizeSquared2D() < 0.1f)
    {
        MoveDirectionAngle = 0.f;
        return;
    }

    FVector DirNorm = Delta.GetSafeNormal2D();
    FVector Fwd     = GetActorForwardVector().GetSafeNormal2D();
    FVector Right   = GetActorRightVector().GetSafeNormal2D();
    MoveDirectionAngle = FMath::RadiansToDegrees(
        FMath::Atan2(FVector::DotProduct(Right, DirNorm),
                     FVector::DotProduct(Fwd,   DirNorm)));
}

void AM1OtherPlayer::OnReceiveAttackSwing(float FacingYaw, uint8 SwingIdx)
{
    bIsAttacking = true;
    SetActorRotation(FRotator(0.f, FacingYaw, 0.f));

    if (GetMoveFlag())
    {
        SetUseUpperBodyWhenMovingFlag(true);
    }

    PlayLeftAttackSectionOnly(SwingIdx);
}

void AM1OtherPlayer::OnReceiveAttackStop()
{
    bIsAttacking = false;
    SetUseUpperBodyWhenMovingFlag(false);
}

void AM1OtherPlayer::OnReceiveSyncPacket(uint64 ServerTimestamp, FVector SyncPosition)
{
    if (SnapshotBuffer.Count == 0) return;

    // 버퍼 초기화 전 마지막 상태 보존
    const FMovementSnapshot& Last = SnapshotBuffer.Last();
    float LastMoveYaw = Last.MoveYaw;
    bool  LastbMoving = Last.bMoving;

    // 기존 상태 전부 초기화
    SnapshotBuffer.Reset();
    bNeedStopCorrection = false;
    StopTargetLocation = FVector::ZeroVector;

    // 위치만 보정된 새 스냅샷 적재
    FMovementSnapshot Snap;
    Snap.ServerTimestamp = ServerTimestamp;
    Snap.Position = SyncPosition;
    Snap.MoveYaw = LastMoveYaw;
    Snap.bMoving = LastbMoving;
    SnapshotBuffer.Add(Snap);
}

void AM1OtherPlayer::UpdateInterpolation(float DeltaTime)
{
    // 보간은 스냅샷 2개가 필요하니 버퍼 비어있거나 1개밖에 없으면 pass
    if (SnapshotBuffer.Count < 2) { ++DbgCount_TooFewSnaps; return; }

    // 시간 동기화 체크(RTT 응답 한번도 못받았으면 보간X)
    if (NetworkManager == nullptr || !(NetworkManager->GetSpawnManager()->GetRTTRecv()))
    { ++DbgCount_NoRTT; return; }

    // RenderTime 계산시 GetServerTimeMs는 클라에서 측정한 UTC값 + 편도 레이턴시 값을 반환함.
    // 편도 레이턴시가 들어가는 이유는 클라는 서버 시간 기준 100ms 전을 렌더링하는 느낌으로 갈건데
    // 서버에서 찍은 stamp가 1000ms이고 편도가 50ms 레이턴시 걸려서 클라가 시간 측정시 1050ms일건데
    // 실제 rendertime은 1050ms + 50ms - 100ms = 950ms로 해야 딱 클라가 stamp찍은 1050ms에서 100ms 전 시간이 나옴
    // 레이턴시가 없으면 rendertime이 900ms가 나올것이고 100ms를 벗어남.
    const uint64 ServerTimeMs   = NetworkManager->GetSpawnManager()->GetServerTimeMs();
    const uint64 DelayMs        = (uint64)SnapShotProc::INTERP_DELAY_MS;
    uint64       RenderTime     = (ServerTimeMs > DelayMs) ? (ServerTimeMs - DelayMs) : 0;

    // ClockOffset 역방향 보정 또는 시스템 클럭 후퇴로 RenderTime이 줄어들면
    // 보간 위치가 역행해서 캐릭터가 뒤로 순간이동하는 현상이 발생하므로 단조 증가 보장
    if (RenderTime < LastRenderTimeMs) RenderTime = LastRenderTimeMs;
    else LastRenderTimeMs = RenderTime;


    // 스냅샷 버퍼의 []에 들어가는 값은 head로 부터 떨어진 offset임.
    // 현재 head에 있는 즉, 가장 오래된 snapshot의 시간값보다 rendertime이 작으면 아직 render할 시간 아니니 리턴
    if (RenderTime < SnapshotBuffer[0].ServerTimestamp)
    { ++DbgCount_TooEarly; return; }

    // 스냅샷 버퍼의 마지막 스냅샷에 찍힌 서버 시간보다 RenderTime이 크면 아직 새로운 패킷이 안온 상태임.
    if (RenderTime >= SnapshotBuffer.Last().ServerTimestamp)
    {
        const FMovementSnapshot& Last = SnapshotBuffer.Last();

        // 마지막 스냅샷이 정지 패킷인데 현재 캐릭터는 이동중이라면 정지 수렴 처리
        if (!Last.bMoving && bMoving)
        {
            ++DbgCount_ExhStop;
            bMoving = false;
            StopTargetLocation = Last.Position;
            StopTargetYaw      = Last.MoveYaw;
            bNeedStopCorrection = true;
        }
        // 이동중인데 패킷이 아직 안 온 경우: 마지막 방향/속도로 외삽(최대 200ms 캡)
        else if (Last.bMoving && bMoving)
        {
            ++DbgCount_ExhExtrap;
            float ElapsedSec = FMath::Min((float)(RenderTime - Last.ServerTimestamp) / 1000.f, 0.2f);
            FVector ForwardDir = FRotator(0.f, Last.MoveYaw, 0.f).Vector();
            FVector ExtrapPos  = Last.Position + ForwardDir * UserConst::WALK_SPEED * ElapsedSec;

            if (!bIsAttacking)
                SetActorLocationAndRotation(ExtrapPos, FRotator(0.f, Last.MoveYaw, 0.f), false, nullptr, ETeleportType::TeleportPhysics);
            else
                SetActorLocation(ExtrapPos, false, nullptr, ETeleportType::TeleportPhysics);
        }
        else
        {
            ++DbgCount_ExhIdle;
        }

        return;
    }

    // 스냅샷 버퍼를 순회
    int32 PrevIdx = -1;
    for (int32 i = 0; i < SnapshotBuffer.Count - 1; i++)
    {
        // 순회하면서 RenderTime을 끼고 있는 2개의 Snapshot을 찾기
        // RenderTime이 1100ms면 ServerTimestamp 1050ms인 스냅샷과 1150ms인 스냅샷을 찾는 것임.
        if (SnapshotBuffer[i].ServerTimestamp <= RenderTime &&
            SnapshotBuffer[i + 1].ServerTimestamp >= RenderTime)
        {
            PrevIdx = i;
            break;
        }
    }

    // 스냅샷을 못찾았으면 리턴
    if (PrevIdx == -1)
    {
        ++DbgCount_NoPair;
        // 타임스탬프 순서가 어긋난 경우 등 원인 파악용 상세 로그 (최초 발생 시마다 출력)
        FString BufDump;
        for (int32 i = 0; i < SnapshotBuffer.Count; ++i)
            BufDump += FString::Printf(TEXT("[%d]%llu(m=%d) "), i, SnapshotBuffer[i].ServerTimestamp, (int32)SnapshotBuffer[i].bMoving);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                (int32)EntityID + 10000,
                5.f,
                FColor::Red,
                FString::Printf(TEXT("[OP %llu] NoPair! RT=%llu Buf=%d | %s"),
                    EntityID, RenderTime, SnapshotBuffer.Count, *BufDump));
        }
        return;
    }


    // RenderTime 이전 스냅샷과 직후 Snapshot을 참조
    const FMovementSnapshot& Prev = SnapshotBuffer[PrevIdx];
    const FMovementSnapshot& Next = SnapshotBuffer[PrevIdx + 1];


    // 이전 Snapshot이 정지 플래그, 다음 스냅샷은 이동 플래그, 현재 해당 캐릭터는 정지한 상태면
    // 이동 시작작업 할 것임.
    // 이동 플래그를 true로 변경하고 수렴 플래그 끄기
    if (!Prev.bMoving && Next.bMoving && !bMoving)
    {
        bMoving = true;
        bNeedStopCorrection = false;
        if (!bIsAttacking)
            SetActorRotation(FRotator(0.f, Next.MoveYaw, 0.f));
        GetCharacterMovement()->MaxWalkSpeed = UserConst::WALK_SPEED;
    }
    else if (Prev.bMoving && Next.bMoving && !bMoving)
    {
        // stop→move 경계를 RenderTime이 이미 지나쳐서 Prev도 이동 스냅샷인 경우
        bMoving = true;
        bNeedStopCorrection = false;
        if (!bIsAttacking)
            SetActorRotation(FRotator(0.f, Prev.MoveYaw, 0.f));
        GetCharacterMovement()->MaxWalkSpeed = UserConst::WALK_SPEED;
    }


    // 이전 스냅샷이 이동 플래그 다음 스냅샷은 정지 플래그 현재 해당 캐릭터가 이동중이라면
    // 이동 플래그 끄고 정지 스냅샷 위치를 정지할 타겟 위치로 설정하고 수렴 플래그 키기
    // 정지 스냅샷의 이동방향에 맞춰서 캐릭터를 그 위치로 수렴시키기 위해 StopTargetYaw도 NextMoveYaw로 설정
    // 바로 다음 함수에서 수렴 처리할것임.
    else if (Prev.bMoving && !Next.bMoving && bMoving)
    {
        bMoving = false;
        StopTargetLocation = Next.Position;
        bNeedStopCorrection = true;
        StopTargetYaw = Next.MoveYaw;
        return;
    }

    // 이동하지 않으면 리턴
    if (!bMoving)
    {
        ++DbgCount_StopNoMove;
        GetCharacterMovement()->MaxWalkSpeed = 0;
        return;
    }

    // 이동한다면 보간 처리


    // 이전 스냅샷을 기준으로 RenderTime이 이전 스냅샷의 서버스탬프와 이후 스냅샷의 서버 스탬프 사이에 어디에 위치해있는지 찾아서 이를 0 ~ 1 소수점으로 정규화
    // 예를 들어 이전 서버 스탬프가 1000ms, Render Time이 1060ms, 이후 서버 스탬프가 1100ms면 대충 RenderTime이 이전, 이후 스냅샷 사이에 60%정도에 위치함.
    float Alpha = (float)(RenderTime - Prev.ServerTimestamp)
        / (float)(Next.ServerTimestamp - Prev.ServerTimestamp);
    Alpha = FMath::Clamp(Alpha, 0.f, 1.f);


    // 이 Alpha 비율값을 가지고 이전 스냅샷의 위치와 이후 스냅샷 위치 사이에서 60% 위치한 위치값 계산
    FVector InterpPos = FMath::Lerp(Prev.Position, Next.Position, Alpha);

    // 방향도 이전 이동방향과 이후 이동방향을 보간함.(공격 안하고 있을 때)
    float DeltaYaw;
    float InterpYaw = GetActorRotation().Yaw;

    if (!bIsAttacking)
    {
        DeltaYaw = FMath::FindDeltaAngleDegrees(Prev.MoveYaw, Next.MoveYaw); // 이전 이동방향과 이후 이동방향 사잇값을 계산
        InterpYaw = Prev.MoveYaw + DeltaYaw * Alpha;                         // 해당 사잇값에 alpha를 곱해서 나온값을 이전 이동방향값에 더해서 보간
    }

    // TeleportPhysics: 매 프레임 velocity를 0으로 초기화해서
    // CharacterMovement가 보간 사이 갭에서 캐릭터를 앞으로 밀어버리는 현상 방지
    ++DbgCount_Rendered;
    SetActorLocationAndRotation(InterpPos, FRotator(0.f, InterpYaw, 0.f), false, nullptr, ETeleportType::TeleportPhysics);

}

void AM1OtherPlayer::UpdateStopCorrection(float DeltaTime)
{
    // 현재 위치에서 정지 위치 사이에 위치값을 Next로 지정해서 거기로 옮기기
    FVector Current = GetActorLocation();
    FVector Next = FMath::VInterpTo(Current, StopTargetLocation, DeltaTime, SnapShotProc::CORRECTION_INTERP_SPEED);

    // 방향도 현재 캐릭터 방향과 정지 스냅샷의 이동방향을 보간해서 서서히 정지 스냅샷의 방향으로 시선 이동하게 함.
    float CurrentYaw;
    float DeltaYaw;
    float InterpYaw;
    
    if (bIsAttacking)
    {
        InterpYaw = GetActorRotation().Yaw;
    }
    else
    {
        CurrentYaw = GetActorRotation().Yaw;
        DeltaYaw = FMath::FindDeltaAngleDegrees(CurrentYaw, StopTargetYaw);
        InterpYaw = CurrentYaw + DeltaYaw * DeltaTime * SnapShotProc::CORRECTION_INTERP_SPEED;
    }

    SetActorLocationAndRotation(Next, FRotator(0.f, InterpYaw, 0.f), false, nullptr, ETeleportType::TeleportPhysics);

    // 만약 다음 이동할 위치가 정지 위치랑 거의 차이없으면 그냥 정지 위치로 이동
    // VInterpTo는 수학적으로 완전히 TargetPos에 도달 못해서 작아지면 그냥 그 위치로 맞춰야 함.
    if (FVector::Dist(Next, StopTargetLocation) < 1.f)
    {
        SetActorLocationAndRotation(StopTargetLocation, FRotator(0.f, StopTargetYaw, 0.f), false, nullptr, ETeleportType::TeleportPhysics);
        bNeedStopCorrection = false;
    }
}

FName AM1OtherPlayer::GetLeftAttackSectionName(uint8 SwingIndex) const
{
    switch (SwingIndex)
    {
    case 1: return TEXT("Attack_A_Slow");
    case 2: return TEXT("Attack_B_Slow");
    case 3: return TEXT("Attack_C_Slow");
    case 4: return TEXT("Attack_D_Slow");
    default: return NAME_None;
    }

}

void AM1OtherPlayer::PlayLeftAttackSectionOnly(uint8 SwingIndex)
{
    if (LeftAttackMontage == nullptr)
        return;

    UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
    if (AnimInst == nullptr)
        return;

    const FName SectionName = GetLeftAttackSectionName(SwingIndex);
    if (SectionName.IsNone())
        return;

    // 몽타주가 재생 중이 아닐 때만 Montage_Play 호출
    // 이미 재생 중인데 Montage_Play(0)을 다시 호출하면 첫 섹션 위치로 리셋된 뒤 점프해서
    // 이전 섹션이 순간적으로 재생되는 문제가 생김
    if (!AnimInst->Montage_IsPlaying(LeftAttackMontage))
    {
        const float Result = AnimInst->Montage_Play(
            LeftAttackMontage,
            1.0f,
            EMontagePlayReturnType::MontageLength,
            0.0f,
            false
        );

        if (Result <= 0.0f)
            return;
    }

    // 해당 섹션으로 이동
    AnimInst->Montage_JumpToSection(SectionName, LeftAttackMontage);

    // 이 섹션 끝나면 다음 섹션으로 넘어가지 말고 종료
    AnimInst->Montage_SetNextSection(
        SectionName,
        NAME_None,
        LeftAttackMontage
    );
}
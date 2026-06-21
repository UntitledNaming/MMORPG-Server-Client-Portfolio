#include "Character/M1OtherPlayer.h"
#include "Network\M1NetworkManager.h"
#include "System\M1SpawnManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "System\M1GameInstance.h"
#include "ContentsDefine.h"
#include "HAL/IConsoleManager.h"

// 이동 동기화 before/after 데모 토글.
//  0 = 스냅샷 보간(after, 부드러움)
//  1 = 추측항법(before): moveflag/yaw만으로 로컬 이동(위치 보정 없음 → 드리프트)
// 게임 중 콘솔(~)에서 "M1.RawMove 1" / "M1.RawMove 0" 으로 전환 → 같은 빌드로 두 영상 촬영.
static TAutoConsoleVariable<int32> CVarOtherPlayerRawMove(
    TEXT("M1.RawMove"),
    0,
    TEXT("0=snapshot interpolation, 1=raw direct-apply (before/after demo)"),
    ECVF_Default);

AM1OtherPlayer::AM1OtherPlayer()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AM1OtherPlayer::ApplySpawnData(const FM1SpawnData& Data)
{
    Super::ApplySpawnData(Data);

    // 스폰 시 MoveFlag로 이동 애니를 미리 켜지 않는다. 위치 궤적(스냅샷)이 쌓여
    // 보간이 실제 이동을 만들 때 bRenderMoving이 파생되며 자연히 켜진다.
    // (미리 켜면 데이터 도착 전까지 '제자리 걷기(treadmill)'가 보임)
    InitOverheadStatus(TEXT("GreyStone"), HP, MaxHP);
    SetOverheadVisible(true);
}

float AM1OtherPlayer::GetMoveSpeed()
{
	// 애니 블렌드스페이스용 속도. 저장 안 하고 파생값으로 제공.
	return bRenderMoving ? UserConst::WALK_SPEED : 0.f;
}

bool  AM1OtherPlayer::GetMoveFlag()
{
	return bRenderMoving;
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

	if (CVarOtherPlayerRawMove.GetValueOnGameThread() == 0)
	{
		UpdateInterpolation(DeltaTime);
	}
	else if (bRenderMoving)
	{
		// before(데모): 추측항법 — 받은 방향(RawMoveYaw)으로 속도만큼 계속 전진.
		// 위치 보정이 없어 시간이 지날수록 실제 위치와 어긋남(드리프트).
		const FVector Dir = FRotator(0.f, RawMoveYaw, 0.f).Vector();
		AddActorWorldOffset(Dir * UserConst::WALK_SPEED * DeltaTime, false);
	}

	UpdateMoveDirection();
	UpdateAttackPose(DeltaTime);
}

void AM1OtherPlayer::UpdateAttackPose(float DeltaTime)
{
    if (!bIsAttacking)
        return;

    AttackPoseRemaining -= DeltaTime;
    if (AttackPoseRemaining > 0.f)
        return;

    // 연타가 끊겨 더 이상 swing 패킷이 오지 않음 → 공격 포즈 해제
    bIsAttacking = false;
    SetUseUpperBodyWhenMovingFlag(false);
}

void AM1OtherPlayer::OnReceiveMovementPacket(const FMovementSnapshot& Snapshot)
{
	// before(데모): 위치(Position)는 안 쓰고 moveflag/yaw만으로 추측항법.
	//  moveflag false→true=이동 시작, true→false=정지. yaw=이동/시선 방향.
	//  Tick에서 RawMoveYaw 방향으로 전진하고, 패킷마다 방향을 갱신(이동 중 방향 변경 반영).
	if (CVarOtherPlayerRawMove.GetValueOnGameThread() != 0)
	{
		bRenderMoving = Snapshot.bMoving;
		if (Snapshot.bMoving)
		{
			RawMoveYaw = Snapshot.MoveYaw;
			if (!bIsAttacking)
				SetActorRotation(FRotator(0.f, RawMoveYaw, 0.f));
		}
		return;
	}

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
    AttackPoseRemaining = AttackPoseHoldSeconds;   // swing 올 때마다 포즈 유지 타이머 갱신
    SetActorRotation(FRotator(0.f, FacingYaw, 0.f));

    if (GetMoveFlag())
    {
        SetUseUpperBodyWhenMovingFlag(true);
    }

    PlayLeftAttackSectionOnly(SwingIdx);
}

void AM1OtherPlayer::OnReceiveSyncPacket(uint64 ServerTimestamp, FVector SyncPosition)
{
    // 서버 강제 위치보정: 히스토리 버리고 보정 위치 1개로 재시작.
    //  → 다음 프레임 보간 분기(가장 오래된/최신 스냅샷 hold)가 이 위치로 스냅.
    //    별도 수렴 로직 불필요.
    const float LastMoveYaw = (SnapshotBuffer.Count > 0) ? SnapshotBuffer.Last().MoveYaw : GetActorRotation().Yaw;
    const bool  LastbMoving = (SnapshotBuffer.Count > 0) ? SnapshotBuffer.Last().bMoving : false;

    SnapshotBuffer.Reset();

    FMovementSnapshot Snap;
    Snap.ServerTimestamp = ServerTimestamp;
    Snap.Position = SyncPosition;
    Snap.MoveYaw = LastMoveYaw;
    Snap.bMoving = LastbMoving;
    SnapshotBuffer.Add(Snap);
}

void AM1OtherPlayer::UpdateInterpolation(float /*DeltaTime*/)
{
    // 진실은 스냅샷 버퍼 하나. 저장된 이동상태(bMoving/정지수렴) 없이 매 프레임 파생한다.
    // 외삽 안 함 → 항상 '과거'를 그리므로 오버슈트/정지수렴이 필요 없다.

    if (SnapshotBuffer.Count == 0) return;

    // 시간 동기화 체크(RTT 응답 한번도 못받았으면 보간X)
    if (NetworkManager == nullptr || !(NetworkManager->GetSpawnManager()->GetRTTRecv()))
        return;

    // 렌더 기준시각 = 서버시간 - 보간지연. (GetServerTimeMs는 클라UTC + 편도레이턴시 반환)
    const uint64 ServerTimeMs = NetworkManager->GetSpawnManager()->GetServerTimeMs();
    const uint64 DelayMs      = (uint64)SnapShotProc::INTERP_DELAY_MS;
    uint64       RenderTime   = (ServerTimeMs > DelayMs) ? (ServerTimeMs - DelayMs) : 0;

    // 클럭 재동기/후퇴로 RenderTime이 줄면 위치가 역행하니 단조 증가 보장
    RenderTime = FMath::Max(RenderTime, LastRenderTimeMs);
    LastRenderTimeMs = RenderTime;

    // (1) 아직 가장 오래된 스냅샷 시각도 안 됐으면: 그 위치에서 대기
    if (RenderTime <= SnapshotBuffer[0].ServerTimestamp)
    {
        ApplyPose(SnapshotBuffer[0].Position);
        bRenderMoving = false;
        return;
    }

    // (2) 최신 스냅샷보다 미래면 버퍼가 마름(정지 후 패킷 끊김 / 레이턴시로 늦게 와 RenderTime이 추월):
    //     외삽하지 않고 '최신(=실제 정지) 위치'에 핀. 늦은 스냅샷도 위치는 살려 어긋남 방지.
    const FMovementSnapshot& Newest = SnapshotBuffer.Last();
    if (RenderTime >= Newest.ServerTimestamp)
    {
        ApplyPose(Newest.Position);
        // 버퍼 마름. 마지막 의도가 이동이면 walk + 이동방향 회전 유지, 정지면 시선 그대로.
        bRenderMoving = Newest.bMoving;
        if (bRenderMoving && !bIsAttacking)
            SetActorRotation(FRotator(0.f, Newest.MoveYaw, 0.f));
        return;
    }

    // (3) RenderTime을 끼는 가장 가까운 두 스냅샷 A(이전)·B(이후)를 찾아 그 사이를 보간.
    //     오래된 스냅샷이 앞에 남아있어도 '가장 타이트한 쌍'을 고르므로 무해.
    int32 i = 0;
    while (i + 1 < SnapshotBuffer.Count &&
           SnapshotBuffer[i + 1].ServerTimestamp <= RenderTime)
    {
        ++i;
    }
    const FMovementSnapshot& A = SnapshotBuffer[i];
    const FMovementSnapshot& B = SnapshotBuffer[i + 1];

    const float Alpha = FMath::Clamp(
        (float)(RenderTime - A.ServerTimestamp) /
        (float)(B.ServerTimestamp - A.ServerTimestamp), 0.f, 1.f);

    // 이동 여부 = 지나는 구간의 양 끝 중 하나라도 이동이면 이동.
    //  - [이동→정지] 구간: 정지점으로 미끄러지는 동안에도 walk 유지(자연스런 감속)
    //  - [정지→이동] 구간: 재이동 첫 순간부터 walk. (예전 freeze가 났던 자리)
    bRenderMoving = (A.bMoving || B.bMoving);

    const FVector Pos = FMath::Lerp(A.Position, B.Position, Alpha);
    ApplyPose(Pos);

    // 이동 중 + 공격 아닐 때만 이동방향으로 회전. 정지(idle)/공격 중엔 현재 시선 유지.
    // (정지 시 MoveYaw로 회전시키면 공격 종료 순간 마지막 이동방향으로 시선이 휙 돌아감)
    if (bRenderMoving && !bIsAttacking)
    {
        const float DeltaYaw = FMath::FindDeltaAngleDegrees(A.MoveYaw, B.MoveYaw);
        const float Yaw      = A.MoveYaw + DeltaYaw * Alpha;
        SetActorRotation(FRotator(0.f, Yaw, 0.f));
    }
}

void AM1OtherPlayer::ApplyPose(const FVector& Pos)
{
    // 위치 전용. 회전은 호출부에서 '이동 중'일 때만 적용(정지/공격 중엔 시선 유지).
    // TeleportPhysics: 매 프레임 velocity를 0으로 만들어 CharacterMovement가
    // 보간 갭에서 캐릭터를 앞으로 밀어버리는 현상 방지.
    SetActorLocation(Pos, false, nullptr, ETeleportType::TeleportPhysics);
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
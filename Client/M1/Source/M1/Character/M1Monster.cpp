#include "M1Monster.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "System\M1GameInstance.h"
#include "ContentsDefine.h"
#include "M1\System\Type\M1Type.h"

AM1Monster::AM1Monster()
{
	PrimaryActorTick.bCanEverTick = true;
}

float AM1Monster::GetMoveSpeed()
{
	return GetCharacterMovement()->MaxWalkSpeed;
}

bool  AM1Monster::GetMoveFlag()
{
	return isMoving;
}

void  AM1Monster::SetHP(int32 NewHP)
{
	Super::SetHP(NewHP);

	SetOverheadHP(NewHP, MaxHP);
}


void AM1Monster::OnReceiveMoveTarget(FMonsterMove& Data)
{
	// 타겟 수정
	m_TargetLocation = Data.TargetLocation;
	m_MoveYaw = Data.MoveYaw;
	isMoving = true;
	GetCharacterMovement()->MaxWalkSpeed = Data.MoveSpeed;

	// 현재 몬스터 위치랑 서버의 몬스터 위치와 크게 차이나면 위치 맞추기
	SetActorRotation(FRotator(0, m_MoveYaw, 0));
	
	float Dist = FVector::Dist2D(GetActorLocation(), Data.MonsterLocation);
	if (Dist >= MonsterConst::POS_SNAP_DIST_CM)
		SetActorLocation(Data.MonsterLocation);

	if (IsNear(GetActorLocation(), Data.TargetLocation))
	{
		isMoving = false;
		SetUseUpperBodyWhenMovingFlag(false);
	}

}

void AM1Monster::OnReceiveAttackTarget()
{
	SetUseUpperBodyWhenMovingFlag(true);

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();

	if (AnimInst && !AnimInst->Montage_IsPlaying(AttackMontage))
	{
		PlayAnimMontage(AttackMontage, MontagePlayRate);

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AM1Monster::OnAttackMontageEnded);
		AnimInst->Montage_SetEndDelegate(EndDelegate, AttackMontage);
	}
}

void AM1Monster::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	SetUseUpperBodyWhenMovingFlag(false);
}


void AM1Monster::BeginPlay()
{
	Super::BeginPlay();

}

void AM1Monster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Move(DeltaTime);
}

void AM1Monster::Move(float DeltaTime)
{
	if (!isMoving)
		return;

	FVector Current = GetActorLocation();

	if (FVector::Dist2D(Current, m_TargetLocation) < 50.0f)
	{
		isMoving = false;
		GetCharacterMovement()->MaxWalkSpeed = 0;
		return;
	}

	FVector MoveDir = FRotator(0.f, m_MoveYaw, 0.f).Vector();
	FVector NewPos = Current + MoveDir * GetCharacterMovement()->MaxWalkSpeed * DeltaTime;

	SetActorLocation(NewPos);
}

bool AM1Monster::IsNear(const FVector& cur, const FVector& target)
{
	float dx = std::abs(cur.X - target.X);
	float dy = std::abs(cur.Y - target.Y);
	float distSq = dx * dx + dy * dy;
	float oneTickDist = GetCharacterMovement()->MaxWalkSpeed / FieldConst::UPDATE_LOOP_TIME;

	float arriveMinDistSq = oneTickDist * oneTickDist;

	if (distSq <= arriveMinDistSq)
		return true;

	return false;
}
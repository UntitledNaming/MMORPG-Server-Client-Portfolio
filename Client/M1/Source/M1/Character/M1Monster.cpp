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
	float Dist = FVector::Dist2D(GetActorLocation(), Data.MonsterLocation);
	if (Dist >= MonsterConst::POS_SNAP_DIST_CM)
	{
		SetActorLocation(Data.MonsterLocation);
		SnapCount++;
		GEngine->AddOnScreenDebugMessage(1, 3.f, FColor::Red,
			FString::Printf(TEXT("Monster PosSnap: %d (dist: %.0fcm)"), SnapCount, Dist));
	}

	m_TargetLocation = Data.TargetLocation;
	isMoving  = true;
	GetCharacterMovement()->MaxWalkSpeed = Data.MoveSpeed;

}

void AM1Monster::OnReceiveAttackTarget(float AttackYaw)
{
	SetUseUpperBodyWhenMovingFlag(true);
	SetActorRotation(FRotator(0, AttackYaw, 0));
	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();

	if (AnimInst && !AnimInst->Montage_IsPlaying(AttackMontage))
	{
		PlayAnimMontage(AttackMontage, MontagePlayRate);

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AM1Monster::OnAttackMontageEnded);
		AnimInst->Montage_SetEndDelegate(EndDelegate, AttackMontage);
	}
}

void AM1Monster::OnReceiveStop(FVector& StopLocation)
{
	float Dist = FVector::Dist2D(GetActorLocation(), StopLocation);

	isMoving = false;
	m_TargetLocation = StopLocation;

	if (Dist < MonsterConst::POS_SNAP_DIST_CM)
		return;
	
	SetActorLocation(FVector(StopLocation.X, StopLocation.Y, GetActorLocation().Z));
	GetCharacterMovement()->MaxWalkSpeed = 0.f;
	return;
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
	FVector Dir     = FVector(m_TargetLocation.X - Current.X, m_TargetLocation.Y - Current.Y, 0.f).GetSafeNormal();
	float   Step    = GetCharacterMovement()->MaxWalkSpeed * DeltaTime;

	FRotator TargetRot = Dir.ToOrientationRotator();
	FRotator NewRot = FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, RotationInterpSpeed);
	SetActorRotation(NewRot);

	if (FVector::Dist2D(Current, m_TargetLocation) <= 50.f)
	{
		isMoving = false;
		GetCharacterMovement()->MaxWalkSpeed = 0.f;
		return;
	}

	SetActorLocation(Current + Dir * Step);
}
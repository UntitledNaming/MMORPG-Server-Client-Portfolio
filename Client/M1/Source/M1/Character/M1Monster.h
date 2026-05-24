// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "M1Character.h"
#include "M1Monster.generated.h"

struct FMonsterMove;

UCLASS()
class M1_API AM1Monster : public AM1Character
{
	GENERATED_BODY()

public:
	AM1Monster();

	virtual float GetMoveSpeed() override;
	virtual bool  GetMoveFlag()  override;
	virtual void  SetHP(int32 NewHP) override;


	void OnReceiveMoveTarget(FMonsterMove& Data);
	void OnReceiveAttackTarget(float AttackYaw);
	void OnReceiveStop(FVector& StopLocation);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void Move(float DeltaTime);
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);


private:
	bool     bIsAttacking     = false;
	bool     isMoving         = false;
	bool     bMovingToStop    = false;
	FVector  m_TargetLocation = FVector::ZeroVector;
	int32    SnapCount        = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<class UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float MontagePlayRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float RotationInterpSpeed = 10.0f;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/M1BasePlayer.h"
#include "M1/System/Type/M1Type.h"
#include "M1OtherPlayer.generated.h"

UCLASS()
class M1_API AM1OtherPlayer : public AM1BasePlayer
{
	GENERATED_BODY()
	
public:
	AM1OtherPlayer();
	virtual float GetMoveSpeed() override;
	virtual bool  GetMoveFlag()  override;
	virtual void  SetHP(int32 NewHP) override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	void OnReceiveMovementPacket(const FMovementSnapshot& Snapshot);
	void OnReceiveSyncPacket(uint64 ServerTimestamp, FVector SyncPosition);
	void OnReceiveAttackSwing(float FacingYaw, uint8 SwingIdx);
	void OnReceiveAttackStop();

	virtual void UpdateMoveDirection() override;

private:
	void UpdateInterpolation(float DeltaTime);
	void UpdateStopCorrection(float DeltaTime);

private:
	TCircularSnapBuffer<FMovementSnapshot, 16> SnapshotBuffer;
	class UM1NetworkManager* NetworkManager = nullptr;

private:
	bool     bIsAttacking        = false;
	bool     bNeedStopCorrection = false;
	float    StopTargetYaw       = 0.f;
	FVector  StopTargetLocation  = FVector::ZeroVector;

	// RenderTime이 역행하지 않도록 보장 (ClockOffset 역방향 보정 / 시스템 클럭 후퇴 방어)
	uint64   LastRenderTimeMs    = 0;

	FVector  PrevLocation        = FVector::ZeroVector;
};

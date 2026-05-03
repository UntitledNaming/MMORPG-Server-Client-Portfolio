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


protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	void OnReceiveMovementPacket(const FMovementSnapshot& Snapshot);
	void OnReceiveSyncPacket(uint64 ServerTimestamp, FVector SyncPosition);

private:
	void UpdateInterpolation(float DeltaTime);
	void UpdateStopCorrection(float DeltaTime);

private:
	TCircularSnapBuffer<FMovementSnapshot, 8> SnapshotBuffer;
	class UM1NetworkManager* NetworkManager = nullptr;

private:	
	bool    bMoving = false;
	bool    bNeedStopCorrection = false;
	float   StopTargetYaw = 0.f;
	FVector StopTargetLocation = FVector::ZeroVector;
};

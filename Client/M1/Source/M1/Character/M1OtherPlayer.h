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
	
	virtual void  ApplySpawnData(const FM1SpawnData& Data) override;
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

	virtual void UpdateMoveDirection() override;
	FName GetLeftAttackSectionName(uint8 SwingIndex) const;
	void PlayLeftAttackSectionOnly(uint8 SwingIndex);
	
private:
	void UpdateInterpolation(float DeltaTime);
	void UpdateAttackPose(float DeltaTime);
	void ApplyPose(const FVector& Pos);   // 위치 전용(회전은 호출부에서 이동 중일 때만)

private:
	TCircularSnapBuffer<FMovementSnapshot, 16> SnapshotBuffer;
	class UM1NetworkManager* NetworkManager = nullptr;

private:
	bool     bIsAttacking        = false;

	// 보간에서 '매 프레임 파생'되는 이동 여부. 전이 판단에 쓰는 저장상태가 아니라
	// 애니메이션으로 내보내는 출력값(읽어서 분기하지 않음 → 상태 동기화 버그 차단)
	bool     bRenderMoving       = false;

	// swing 패킷이 끊기면 공격 포즈를 자가 해제하기 위한 타이머 (stop 프로토콜 대체)
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float    AttackPoseHoldSeconds = 1.2f;   // swing 1타 간격보다 약간 길게
	float    AttackPoseRemaining   = 0.f;

	// RenderTime이 역행하지 않도록 보장 (ClockOffset 역방향 보정 / 시스템 클럭 후퇴 방어)
	uint64   LastRenderTimeMs    = 0;

	FVector  PrevLocation        = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<class UAnimMontage> LeftAttackMontage;
};

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
	FName GetLeftAttackSectionName(uint8 SwingIndex) const;
	void PlayLeftAttackSectionOnly(uint8 SwingIndex);
	
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

	// 보간 스킵 원인 진단용 카운터 (2초마다 로그 출력 후 리셋)
	float  DbgLogTimer          = 0.f;
	int32  DbgCount_TooFewSnaps = 0;  // 버퍼 스냅샷 2개 미만
	int32  DbgCount_NoRTT       = 0;  // RTT 동기화 미완료
	int32  DbgCount_TooEarly    = 0;  // RenderTime < 가장 오래된 스냅샷
	int32  DbgCount_ExhStop     = 0;  // 버퍼 고갈 + 정지 스냅샷 → 정지 수렴 전환
	int32  DbgCount_ExhExtrap   = 0;  // 버퍼 고갈 + 이동 스냅샷 → 외삽 처리
	int32  DbgCount_ExhIdle     = 0;  // 버퍼 고갈 + 이미 정지 → 대기
	int32  DbgCount_NoPair      = 0;  // 버퍼 순회했는데 RenderTime 끼운 쌍 못 찾음
	int32  DbgCount_StopNoMove  = 0;  // 이동 안 해서 리턴
	int32  DbgCount_Rendered    = 0;  // 정상 보간 완료

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<class UAnimMontage> LeftAttackMontage;
};

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "M1/System/Type/M1Type.h"
#include "Ability\M1AbilityTypes.h"
#include "M1SpawnManager.generated.h"

class CMessage;

UCLASS()
class M1_API AM1SpawnManager : public AActor
{
	GENERATED_BODY()
	
	struct FOffsetSample { int64 Offset; uint64 RTT_ms; };

public:	
	AM1SpawnManager();

	static uint64 GetLocalTimeMs()
	{
		FDateTime Now = FDateTime::UtcNow();
		return (uint64)Now.ToUnixTimestamp() * 1000 + (uint64)Now.GetMillisecond();
	}

	uint64 GetServerTimeMs() const
	{
		return GetLocalTimeMs() + ClockOffsetMs;
	}

	bool GetRTTRecv() const
	{
		return bSynced;
	}

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float Deltatime) override;

public:
	void SpawnMyPlayer(FM1SpawnData& Data);
	void SpawnOtherPlayer(FM1SpawnData& Data);
	void SpawnMonster(FM1SpawnData& Data);
	void DespawnPlayer(uint64 EntityID);
	void DespawnMonster(uint64 EntityID);
	void SyncMyPlayer(FVector& Location);
	void SyncOtherPlayer(uint64 EntityID, FVector& Location, uint64 ServerTimestamp);
	void UpdateOtherPlayerMovementInput(uint64 EntityID, FMovementSnapshot& Snapshot);
	void OnOtherPlayerAttackSwing(uint64 EntityID, float FacingYaw, uint8 SwingIdx);
	void OnOtherPlayerAttackStop(uint64 EntityID);
	void ProcessClientAttackHit(FVector Origin, FVector Forward, float Range, float HalfAngleDeg);
	void ProcessClientSingleTargetHit(FVector Origin, FVector Forward, float Range, float HalfAngleDeg);
	void ApplyPlayerHitResult(uint64 EntityID, int32 NewHP);
	void ApplyMonsterHitResult(uint64 EntityID, int32 NewHP);
	void OnMyPlayerSkillResponse(EAbilitySlot Slot, bool bSuccess);
	void OnOtherCharacterUseSkill(uint64 EntityID, EAbilitySlot Slot);
	void OnMonsterMove(uint64 EntityID, FMonsterMove& Data);
	void OnMonsterAttack(uint64 EntityID, float AttackYaw);

	class AM1Character* FindPlayer(uint64 EntityID) const;
	class AM1Character* FindMonster(uint64 EntityID) const;

	// RTT 측정 관련 함수
	void SendRttPacket();
	void mpCreateRttPacket(CMessage* pMessage, double Time);
	void GetRTTEchoMsg(uint64 ServerStampMs);
	void mpMovementInput(CMessage* pMessage, const FVector& Location, float Yaw, bool MoveFlag);
	void TickClock(float DeltaTime);

private:
	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<class AM1LocalPlayer>              LocalPlayerCharacterClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<class AM1OtherPlayer>              OtherPlayerCharacterClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<class AM1Monster>                  MonsterCharacterClass;

	UPROPERTY()
	TObjectPtr<AM1LocalPlayer>                     MyPlayer = nullptr;

	UPROPERTY()
	TMap<uint64, class AM1OtherPlayer*>            PlayerMap;  // 타 플레이어 관리 자료구조

	UPROPERTY()
	TMap<uint64, class AM1Monster*>                MonsterMap;     // 몬스터 관리 자료구조

private:
	uint64                   MyID = 0;                       // 서버로 부터 부여받은 내 캐릭터 ID
	int64                    ClockOffsetMs = 0;              // 현재 적용 중인 offset (점진 보정 대상)
	int64                    TargetOffsetMs = 0;             // 새 측정으로 갱신되는 목표 offset
	uint64                   PendingPingSentTime = 0;        // 핑 송신 시점 (로컬 ms)
	TArray<FOffsetSample>    OffsetSamples;                  // 최근 측정 샘플 N개
	class UM1NetworkManager* NetworkManager = nullptr;
	double                   OldRTTCheckTime = 0.f;		     
	bool                     bSynced = false;                // 첫 동기화 완료 여부
};

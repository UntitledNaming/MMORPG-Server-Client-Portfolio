#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "M1/System/Type/M1Type.h"
#include "M1SpawnManager.generated.h"

class CMessage;

UCLASS()
class M1_API AM1SpawnManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AM1SpawnManager();

	static uint64 GetLocalTimeMs()
	{
		FDateTime Now = FDateTime::UtcNow();
		return (uint64)Now.ToUnixTimestamp() * 1000 + (uint64)Now.GetMillisecond();
	}

	uint64 GetServerTimeMs() const
	{
		return GetLocalTimeMs() + (uint64)(RTT * 1000.0) / 2;
	}

	bool GetRTTRecv() const
	{
		return RTTRecv;
	}

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float Deltatime) override;

public:
	void SpawnMyPlayer(FM1SpawnData& Data);
	void SpawnOtehrPlayer(FM1SpawnData& Data);
	void SpawnMonster(FM1SpawnData& Data);
	void DespawnPlayer(uint64 EntityID);
	void DespawnMonster(uint64 EntityID);
	void SyncMyPlayer(FVector& Location);
	void SyncOtherPlayer(uint64 EntityID, FVector& Location, uint64 ServerTimestamp);
	void UpdateOtherPlayerMovementInput(uint64 EntityID, FMovementSnapshot& Snapshot);

	class AM1Character* FindPlayer(uint64 EntityID) const;
	class AM1Character* FindMonster(uint64 EntityID) const;

	// RTT 측정 관련 함수
	void SendRttPacket();
	void mpCreateRttPacket(CMessage* pMessage, double Time);
	void GetRTTEchoMsg();
	void mpMovementInput(CMessage* pMessage, const FVector& Location, float Yaw, bool MoveFlag);

private:
	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<class AM1LocalPlayer> LocalPlayerCharacterClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<class AM1OtherPlayer> OtherPlayerCharacterClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<class AM1Monster> MonsterCharacterClass;

	UPROPERTY()
	TObjectPtr<AM1LocalPlayer> MyPlayer = nullptr;

	UPROPERTY()
	TMap<uint64, class AM1OtherPlayer*> PlayerMap;  // 타 플레이어 관리 자료구조

	UPROPERTY()
	TMap<uint64, class AM1Monster*> MonsterMap;     // 몬스터 관리 자료구조

	bool   RTTRecv = false;
	double LastSendTime = 0;
	double OldRTTCheckTime = 0;
	double RTT = 0;
	class UM1NetworkManager* NetworkManager = nullptr;
};

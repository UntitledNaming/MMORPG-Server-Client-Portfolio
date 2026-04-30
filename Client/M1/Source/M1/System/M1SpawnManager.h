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
	void SyncOtherPlayer(uint64 EntityID, FVector& Location);
	void UpdateOtherPlayerMovementInput(uint64 EntityID, FVector& Location, FRotator& Rotation, bool Moveflag);

	class AM1Character* FindPlayer(uint64 EntityID) const;
	class AM1Character* FindMonster(uint64 EntityID) const;

	// todo : 플레이어, 몬스터 상태 업데이트
	//void UpdatePlayerState(uint64 EntityID, const FVector& Location, const FRotator& Rotation, EM1ActionStateType ActionType, uint8 InputMask, int32 HP, int32 MaxHP);
	//void UpdateMonsterState(uint64 EntityID, const FVector& Location, const FRotator& Rotation, EM1ActionStateType ActionType,int32 HP, int32 MaxHP);

	// RTT 측정 관련 함수
	void SendRttPacket();
	void mpCreateRttPacket(CMessage* pMessage, double Time);
	void GetRTTEchoMsg();
	void mpMovementInput(CMessage* pMessage, const FVector& Location, float Yaw, bool MoveFlag);

private:
	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<class AM1Player> PlayerCharacterClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<class AM1Monster> MonsterCharacterClass;

	UPROPERTY()
	TObjectPtr<AM1Player> MyPlayer = nullptr;

	UPROPERTY()
	TMap<uint64, class AM1Player*> PlayerMap;  // 나와 타 플레이어 관리 자료구조

	UPROPERTY()
	TMap<uint64, class AM1Monster*> MonsterMap; // 몬스터 관리 자료구조


	double LastSendTime = 0;
	double OldRTTCheckTime = 0;
	class UM1NetworkManager* NetworkManager = nullptr;
};

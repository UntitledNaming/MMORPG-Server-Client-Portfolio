#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "M1/System/Type/M1Type.h"
#include "M1SpawnManager.generated.h"

UCLASS()
class M1_API AM1SpawnManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AM1SpawnManager();

protected:
	virtual void BeginPlay() override;

public:
	void SpawnMyPlayer(const FM1SpawnData& Data);
	void SpawnOtehrPlayer(const FM1SpawnData& Data);
	void SpawnMonster(const FM1SpawnData& Data);
	void DespawnEntity(uint64 EntityID);

	class AM1Character* FindPlayer(uint64 EntityID) const;
	class AM1Character* FindMonster(uint64 EntityID) const;

	// todo : 플레이어, 몬스터 상태 업데이트
	void UpdatePlayerState(uint64 EntityID, const FVector& Location, const FRotator& Rotation, EM1ActionType ActionType, uint8 InputMask, int32 HP, int32 MaxHP);
	void UpdateMonsterState(uint64 EntityID, const FVector& Location, const FRotator& Rotation, EM1ActionType ActionType,int32 HP, int32 MaxHP);

private:
	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<class AM1Player> PlayerCharacterClass;

	//UPROPERTY(EditAnywhere, Category = "Spawn")
	//TSubclassOf<class AM1MonsterCharacter> MonsterCharacterClass;

	UPROPERTY()
	TMap<uint64, class AM1Character*> PlayerMap;  // 나와 타 플레이어 관리 자료구조

	UPROPERTY()
	TMap<uint64, class AM1Character*> MonsterMap; // 몬스터 관리 자료구조
};

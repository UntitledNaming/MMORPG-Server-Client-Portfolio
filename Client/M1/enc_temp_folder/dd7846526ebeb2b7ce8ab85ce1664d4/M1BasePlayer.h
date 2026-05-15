#pragma once

#include "CoreMinimal.h"
#include "M1Character.h"
#include "M1BasePlayer.generated.h"

// MP, EXP 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnManaChanged, int32, CurrentMana, int32, MaxMana);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExpChanged, float, CurrentExp, float, RequiredExp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUp, int32, NewLevel);

UCLASS()
class M1_API AM1BasePlayer : public AM1Character
{
	GENERATED_BODY()
	
public:
	AM1BasePlayer();

protected:
	virtual void BeginPlay() override;

public:
	virtual void ApplySpawnData(const FM1SpawnData& Data) override;

	FORCEINLINE uint8 GetActionType() { return ActionType; }
	FORCEINLINE uint8 GetMoveMode() { return MoveMode; }

public:
	// Getter
	FORCEINLINE float GetCurrentMana() const { return MP; }
	FORCEINLINE float GetMaxMana() const { return MaxMP; }
	FORCEINLINE float GetManaPercent() const { return MaxMP > 0 ? MP / MaxMP : 0.0f; }
	FORCEINLINE float GetCurrentExp() const { return CurrentExp; }
	FORCEINLINE float GetRequiredExp() const { return RequiredExp; }
	FORCEINLINE float GetExpPercent() const { return RequiredExp > 0 ? CurrentExp / RequiredExp : 0.0f; }
	FORCEINLINE int32 GetLevel() const { return Level; }

private:
	void LevelUp();

public:
	// 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnManaChanged OnManaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnExpChanged  OnExpChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLevelUp     OnLevelUpdate;

protected:
	UPROPERTY(VisibleAnywhere) FString NickName;

	UPROPERTY()   uint16   MP = 0;
	UPROPERTY()   uint16   MaxMP = 0;
	UPROPERTY()   uint16   MPRegenPerSec = 0;

	UPROPERTY()   uint16   Level = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")   float RequiredExp = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats") float CurrentExp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")   float ExpMultiplier = 1.5f; // 레벨업 시 필요 경험치 배율

	UPROPERTY(VisibleAnywhere)                                        uint8   MoveMode = 0;
							                                          
	UPROPERTY(VisibleAnywhere)                                        uint8   ActionType = 0;
							                                          
	UPROPERTY(VisibleAnywhere)                                        bool    bMoving = false;

protected:

};

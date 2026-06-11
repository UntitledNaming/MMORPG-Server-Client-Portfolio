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

public:
	// Getter
	FORCEINLINE int16 GetCurrentMana() const { return MP; }
	FORCEINLINE int16 GetMaxMana() const { return MaxMP; }
	FORCEINLINE int16 GetManaPercent() const { return MaxMP > 0 ? MP / MaxMP : 0; }
	FORCEINLINE int32 GetCurrentExp() const { return CurrentExp; }
	FORCEINLINE int32 GetRequiredExp() const { return RequiredExp; }
	FORCEINLINE float GetExpPercent() const { return RequiredExp > 0 ? CurrentExp / RequiredExp : 0.0f; }
	FORCEINLINE uint16 GetLevel() const { return Level; }
	void SetCurrentMana(uint16 Mana);
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

	UPROPERTY()   int16    MP = 0;
	UPROPERTY()   int16    MaxMP = 0;
	UPROPERTY()   int16    MPRegenPerSec = 0;
	UPROPERTY()   uint16   Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")   int32 RequiredExp = 100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats") int32 CurrentExp = 0;

	UPROPERTY(VisibleAnywhere)                                        bool    bMoving = false;

protected:

};

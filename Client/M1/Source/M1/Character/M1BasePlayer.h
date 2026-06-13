#pragma once

#include "CoreMinimal.h"
#include "M1Character.h"
#include "ContentsDefine.h"
#include "M1BasePlayer.generated.h"

// MP, EXP 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnManaChanged, int32, CurrentMana, int32, MaxMana);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExpChanged, float, CurrentExp, float, RequiredExp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUp, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnStatChanged, int32, ATK, int32, DEF, int32, MaxHP, int32, MaxMP, int32, HPRegen, int32, MPRegen);

constexpr FM1LevelData LevelTable[UserConst::USER_MAX_LEVEL + 1] =
{
	{ {},                              0    },
	{ { 12, 3, 130, 100, 1, 3 },      100  },
	{ { 14, 3, 150, 110, 1, 4 },      150  },
	{ { 16, 4, 170, 120, 2, 4 },      230  },
	{ { 18, 4, 190, 130, 2, 5 },      350  },
	{ { 20, 5, 210, 140, 2, 5 },      520  },
	{ { 22, 5, 230, 150, 3, 6 },      750  },
	{ { 24, 6, 250, 160, 3, 6 },      1050 },
	{ { 26, 6, 270, 170, 3, 7 },      1450 },
	{ { 28, 7, 290, 180, 4, 7 },      1950 },
	{ { 30, 7, 310, 190, 4, 8 },      2600 },
};


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
	FORCEINLINE int16            GetCurrentMana() const { return MP; }
	FORCEINLINE int16            GetMaxMana() const { return MaxMP; }
	FORCEINLINE int16            GetManaPercent() const { return MaxMP > 0 ? MP / MaxMP : 0; }
	FORCEINLINE int32            GetCurrentExp() const { return CurrentExp; }
	FORCEINLINE int32            GetRequiredExp() const { return RequiredExp; }
	FORCEINLINE float            GetExpPercent() const { return RequiredExp > 0 ? CurrentExp / RequiredExp : 0.0f; }
	FORCEINLINE uint16           GetLevel() const { return Level; }
	            void             SetCurrentMana(uint16 Mana);
				void             SetCurrentExp(uint32 NewExp);
	            void             SetLevelStat(uint16 NewLevel);
	            void             SetEquipStat(const FM1CharacterStat& NewEquipStat);
	            void             ApplyBuff();
	            void             ClearBuff();
	            void             OnLevelUpData(uint16 NewLevel, uint32 NewCurrentExp);
	            FM1CharacterStat GetFinalStat() const { return BaseStat + EquipStat + BuffStat; }
	            int16            GetATK()       const { return GetFinalStat().ATK; }
	            int16            GetDEF()       const { return GetFinalStat().DEF; }

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStatChanged   OnStatChanged;

protected:
	FM1CharacterStat BaseStat;             // Level     Base Stat
	FM1CharacterStat EquipStat;            // Equipment Base Stat
	FM1CharacterStat BuffStat;             // Buff           Stat
	FTimerHandle     BuffTimerHandle;

private:
	void LevelUp();
	void RecalculateFinalStat();

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
	UPROPERTY()   int16    HPRegenPerSec = 0;  // 추가
	UPROPERTY()   uint16   Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")   int32   RequiredExp = 100;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats") int32   CurrentExp = 0;
	UPROPERTY(VisibleAnywhere)                                        bool    bMoving = false;

protected:

};

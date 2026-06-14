#pragma once

#include "CoreMinimal.h"
#include "M1Character.h"
#include "ContentsDefine.h"
#include "M1BasePlayer.generated.h"

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

public:
	virtual void ApplySpawnData(const FM1SpawnData& Data) override;

public:
	// Getter
	FORCEINLINE int16            GetCurrentMana() const { return MP; }
	FORCEINLINE int16            GetMaxMana() const { return MaxMP; }
	FORCEINLINE float            GetManaPercent() const { return MaxMP > 0 ? (float)MP / MaxMP : 0; }
	FORCEINLINE int32            GetCurrentExp() const { return CurrentExp; }
	FORCEINLINE int32            GetRequiredExp() const { return RequiredExp; }
	FORCEINLINE float            GetExpPercent() const { return RequiredExp > 0 ? (float)CurrentExp / (float)RequiredExp : 0.0f; }
	FORCEINLINE uint16           GetLevel() const { return Level; }

	    virtual void             SetCurrentMana(uint16 Mana);
		virtual void             SetCurrentExp(uint32 NewExp);
		virtual	void             OnLevelUpData(uint16 NewLevel, uint32 NewCurrentExp);

	            void             SetLevelStat(uint16 NewLevel);
	            void             SetEquipStat(const FM1CharacterStat& NewEquipStat);
	            void             ApplyBuff();
	            void             ClearBuff();

	            FM1CharacterStat GetFinalStat() const { return BaseStat + EquipStat + BuffStat; }
	            int16            GetATK()       const { return GetFinalStat().ATK; }
	            int16            GetDEF()       const { return GetFinalStat().DEF; }

protected:
	virtual void OnFinalStatChanged(const FM1CharacterStat& FinalStat) {};

private:
	void RecalculateFinalStat();

protected:
	FM1CharacterStat BaseStat;             // Level     Base Stat
	FM1CharacterStat EquipStat;            // Equipment Base Stat
	FM1CharacterStat BuffStat;             // Buff           Stat
	FTimerHandle     BuffTimerHandle;

protected:
	UPROPERTY(VisibleAnywhere) FString NickName;

	UPROPERTY()                int16    MP = 0;
	UPROPERTY()                int16    MaxMP = 0;
	UPROPERTY()                int16    MPRegenPerSec = 0;
	UPROPERTY()                int16    HPRegenPerSec = 0; 
	UPROPERTY()                uint16   Level = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")   int32   RequiredExp = 100;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats") int32   CurrentExp = 0;
	UPROPERTY(VisibleAnywhere)                                        bool    bMoving = false;
};

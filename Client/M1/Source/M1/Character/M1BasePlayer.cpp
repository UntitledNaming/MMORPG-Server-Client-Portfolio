#include "Character/M1BasePlayer.h"


AM1BasePlayer::AM1BasePlayer()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AM1BasePlayer::ApplySpawnData(const FM1SpawnData& Data)
{
	Super::ApplySpawnData(Data);
	MP = Data.MP;
	CurrentExp = Data.CurrentEXP;
	Level = Data.Level;
	bMoving = Data.MoveFlag;
}

void  AM1BasePlayer::SetCurrentMana(uint16 Mana)
{
	MP = FMath::Clamp(Mana, 0, MaxMP);
}

void  AM1BasePlayer::SetCurrentExp(uint32 NewExp)
{
	CurrentExp = NewExp;
}

void AM1BasePlayer::SetLevelStat(uint16 NewLevel)
{
	if (NewLevel == 0 || NewLevel > UserConst::USER_MAX_LEVEL)
		return;

	Level = NewLevel;
	BaseStat = LevelTable[Level].Stat;
	RequiredExp = LevelTable[Level].RequiredExp;
	RecalculateFinalStat();
}

void AM1BasePlayer::SetEquipStat(const FM1CharacterStat& NewEquipStat)
{
	EquipStat = NewEquipStat;
	RecalculateFinalStat();
}

void AM1BasePlayer::ApplyBuff()
{
	BuffStat.ATK = ClientAttack::BUFF_ATK_ADD_AMOUNT;
	BuffStat.DEF = ClientAttack::BUFF_DEF_ADD_AMOUNT;

	RecalculateFinalStat();

	GetWorldTimerManager().SetTimer(
		BuffTimerHandle,
		this,
		&AM1BasePlayer::ClearBuff,
		ClientAttack::BUFF_DURATION / 1000.f,
		false
	);
}

void AM1BasePlayer::ClearBuff()
{
	BuffStat = FM1CharacterStat{};
	RecalculateFinalStat();
}

void AM1BasePlayer::OnLevelUpData(uint16 NewLevel, uint32 NewCurrentExp)
{
	SetLevelStat(NewLevel);
	CurrentExp = NewCurrentExp;
}

void AM1BasePlayer::RecalculateFinalStat()
{
	FM1CharacterStat Final = BaseStat + EquipStat + BuffStat;

	MaxHP = Final.MaxHP;
	HP = FMath::Clamp((int32)HP, 0, (int32)MaxHP);
	HPRegenPerSec = Final.HPRegenPerSec;

	MaxMP = Final.MaxMP;
	MP = FMath::Clamp((int32)MP, 0, (int32)MaxMP);
	MPRegenPerSec = Final.MPRegenPerSec;

	OnFinalStatChanged(Final);
}
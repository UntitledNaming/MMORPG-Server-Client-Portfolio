// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/M1Ability_Buff.h"
#include "ContentsDefine.h"
#include "Character\M1Character.h"
#include "Character\M1BasePlayer.h"
#include "Controller\M1PlayerController.h"

UM1Ability_Buff::UM1Ability_Buff()
{
	CoolTime = ClientAttack::BUFF_COOLTIME_SEC / 1000;
	RequiredMP = ClientAttack::BUFF_REQUIRED_MANA;
}

void UM1Ability_Buff::OnActivate(AM1Character* Owner)
{
	AM1BasePlayer* Player = Cast<AM1BasePlayer>(Owner);
	if (Player == nullptr)
		return;

	if (Player->GetCurrentMana() < RequiredMP)
		return;

	AM1PlayerController* PC = Cast<AM1PlayerController>(Player->GetController());
	if (!PC) 
		return;

	PC->SendUseSkillPacket(static_cast<uint8>(EAbilitySlot::Skill1) - 1);

}

void UM1Ability_Buff::OnServerConfirmed(AM1Character* Owner)
{
	AM1BasePlayer* Player = Cast<AM1BasePlayer>(Owner);
	if (Player == nullptr)
		return;

	Player->SetUseUpperBodyWhenMovingFlag(true);

	PlayCastFX(Player);

	uint16 mana = Player->GetCurrentMana();
	mana -= RequiredMP;

	Player->SetCurrentMana(mana);
	Player->ApplyBuff();
}

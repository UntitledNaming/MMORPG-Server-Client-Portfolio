// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/M1Ability_DefencBuff.h"
#include "ContentsDefine.h"
#include "Character\M1Character.h"
#include "Character\M1BasePlayer.h"
#include "Controller\M1PlayerController.h"

UM1Ability_DefencBuff::UM1Ability_DefencBuff()
{
	Cooldown = ClientAttack::DEFENCE_BUFF_COOLTIME_SEC;
	RequiredMP = ClientAttack::DEFENCE_BUFF_REQUIRED_MANA;
}

void UM1Ability_DefencBuff::OnActivate(AM1Character* Owner)
{
	AM1BasePlayer* Player = Cast<AM1BasePlayer>(Owner);
	if (Player == nullptr)
		return;

	if (Player->GetCurrentMana() < RequiredMP)
		return;

	AM1PlayerController* PC = Cast<AM1PlayerController>(Player->GetController());
	if (!PC) 
		return;

	PC->SendUseSkillPacket(static_cast<uint8>(EAbilitySlot::Skill1));

}

void UM1Ability_DefencBuff::OnServerConfirmed(AM1Character* Owner)
{
	AM1BasePlayer* Player = Cast<AM1BasePlayer>(Owner);
	if (Player == nullptr)
		return;

	Player->SetUseUpperBodyWhenMovingFlag(true);

	PlayCastFX(Player);

	int mana = Player->GetCurrentMana();
	mana -= RequiredMP;

	Player->SetCurrentMana(mana);
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/M1Ability_DefencBuff.h"
#include "ContentsDefine.h"

UM1Ability_DefencBuff::UM1Ability_DefencBuff()
{
	Cooldown = ClientAttack::DEFENCE_BUFF_COOLTIME_SEC;
	RequiredMP = ClientAttack::DEFENCE_BUFF_REQUIRED_MANA;
}

void UM1Ability_DefencBuff::OnServerConfirmed(AM1Character* Owner)
{

}
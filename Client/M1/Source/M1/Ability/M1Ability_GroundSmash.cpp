#include "Ability/M1Ability_GroundSmash.h"
#include "ContentsDefine.h"
#include "Character\M1Character.h"
#include "Character\M1BasePlayer.h"
#include "Controller\M1PlayerController.h"

UM1Ability_GroundSmash::UM1Ability_GroundSmash()
{
	CoolTime = ClientAttack::GROUNDSMASH_COOLTIME_SEC / 1000;
	RequiredMP = ClientAttack::GROUNDSMASH_REQUIRED_MANA;
	FXData.CastEffectRadius = ClientAttack::GROUNDSMASH_RANGE;
}

void UM1Ability_GroundSmash::OnActivate(AM1Character* Owner)
{
	AM1BasePlayer* Player = Cast<AM1BasePlayer>(Owner);
	if (Player == nullptr)
		return;

	if (Player->GetCurrentMana() < RequiredMP)
		return;

	AM1PlayerController* PC = Cast<AM1PlayerController>(Player->GetController());
	if (!PC)
		return;

	PC->SendUseSkillPacket(static_cast<uint8>(EAbilitySlot::Skill3) - 1);
}

void UM1Ability_GroundSmash::OnServerConfirmed(AM1Character* Owner)
{
	AM1BasePlayer* Player = Cast<AM1BasePlayer>(Owner);
	if (Player == nullptr)
		return;

	Player->SetUseUpperBodyWhenMovingFlag(false);

	PlayCastFX(Player);

	uint16 mana = Player->GetCurrentMana();
	mana -= RequiredMP;

	Player->SetCurrentMana(mana);
}


#include "Ability/M1Ability_SpinSmash.h"
#include "ContentsDefine.h"
#include "Character\M1Character.h"
#include "Character\M1BasePlayer.h"
#include "Controller\M1PlayerController.h"


UM1Ability_SpinSmash::UM1Ability_SpinSmash()
{
	CoolTime = ClientAttack::SPINSLASH_COOLTIME_SEC / 1000;
	RequiredMP = ClientAttack::DEFENCE_BUFF_REQUIRED_MANA;
}

void UM1Ability_SpinSmash::OnActivate(AM1Character* Owner)
{
	AM1BasePlayer* Player = Cast<AM1BasePlayer>(Owner);
	if (Player == nullptr)
		return;

	if (Player->GetCurrentMana() < RequiredMP)
		return;

	AM1PlayerController* PC = Cast<AM1PlayerController>(Player->GetController());
	if (!PC)
		return;

	PC->SendUseSkillPacket(static_cast<uint8>(EAbilitySlot::Skill2));

}

void UM1Ability_SpinSmash::OnServerConfirmed(AM1Character* Owner)
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

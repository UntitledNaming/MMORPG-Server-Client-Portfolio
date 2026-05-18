
#include "Ability/M1Ability_SpinSmash.h"
#include "ContentsDefine.h"
#include "Character\M1Character.h"
#include "Character\M1BasePlayer.h"
#include "Controller\M1PlayerController.h"
#include "Network\M1NetworkManager.h"
#include "System\M1SpawnManager.h"


UM1Ability_SpinSmash::UM1Ability_SpinSmash()
{
	CoolTime = ClientAttack::SPINSLASH_COOLTIME_SEC / 1000;
	RequiredMP = ClientAttack::SPINSLASH_REQUIRED_MANA;
	FXData.CastEffectRadius = ClientAttack::SPINSLASH_RANGE;
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

	PC->SendUseSkillPacket(static_cast<uint8>(EAbilitySlot::Skill2) - 1);

}

void UM1Ability_SpinSmash::OnServerConfirmed(AM1Character* Owner)
{
	AM1BasePlayer* Player = Cast<AM1BasePlayer>(Owner);
	if (Player == nullptr)
		return;

	Player->SetUseUpperBodyWhenMovingFlag(true);

	PlayCastFX(Player);

	if (UM1NetworkManager* NM = GetNetworkManager(Owner))
	{
		if (AM1SpawnManager* SM = NM->GetSpawnManager())
			SM->ProcessClientAttackHit(Owner->GetActorLocation(), Owner->GetActorForwardVector(), ClientAttack::SPINSLASH_RANGE, 180.f);
	}

	uint16 mana = Player->GetCurrentMana();
	mana -= RequiredMP;

	Player->SetCurrentMana(mana);
}

#include "Ability/M1Ability_ServerValidated.h"
#include "Character\M1Character.h"
#include "Controller\M1PlayerController.h"

void UM1Ability_ServerValidated::OnActivate(AM1Character* Owner)
{
    if (!Owner || bIsPending) return;
    bIsPending = true;

    AM1PlayerController* PC = Cast<AM1PlayerController>(Owner->GetController());
    if (PC)
        PC->SendUseSkillPacket(static_cast<uint8>(Slot));
}

void UM1Ability_ServerValidated::OnServerConfirmed(AM1Character* Owner)
{
    bIsPending = false;
    PlayCastFX(Owner);
}

void UM1Ability_ServerValidated::OnServerRejected(AM1Character* Owner)
{
    bIsPending = false;
}

void UM1Ability_ServerValidated::OnDeactivate(AM1Character* Owner)
{
    bIsPending = false;
}

#include "Ability/M1PlayerActorComponent.h"
#include "Data\M1CharacterAbilityDataAsset.h"
#include "Ability\M1AbilityBase.h"
#include "Ability\M1Ability_BasicAttack.h"
#include "Character\M1Character.h"


UM1PlayerActorComponent::UM1PlayerActorComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UM1PlayerActorComponent::InitializeFromData(UM1CharacterAbilityDataAsset* Data)
{
    if (!Data) return;
    AbilityInstances.Empty();

    for (auto& Pair : Data->AbilityEntries)
    {
        if (!Pair.Value.AbilityClass) continue;
        UM1AbilityBase* Instance = NewObject<UM1AbilityBase>(this, Pair.Value.AbilityClass);
        Instance->Slot   = Pair.Key;
        Instance->FXData = Pair.Value.FXData;
        AbilityInstances.Add(Pair.Key, Instance);
    }
}

void UM1PlayerActorComponent::ActivateAbility(EAbilitySlot Slot)
{
    if (UM1AbilityBase* Ability = AbilityInstances.FindRef(Slot))
        Ability->OnActivate(GetOwnerCharacter());
}

void UM1PlayerActorComponent::DeactivateAbility(EAbilitySlot Slot)
{
    if (UM1AbilityBase* Ability = AbilityInstances.FindRef(Slot))
        Ability->OnDeactivate(GetOwnerCharacter());
}

void UM1PlayerActorComponent::OnSkillResponse(EAbilitySlot Slot, bool bSuccess)
{
    if (UM1AbilityBase* Ability = AbilityInstances.FindRef(Slot))
    {
        if (bSuccess)
            Ability->OnServerConfirmed(GetOwnerCharacter());
        else
            Ability->OnServerRejected(GetOwnerCharacter());
    }
}

// 타 캐릭터가 스킬 쓸 때 발동하는 함수
void UM1PlayerActorComponent::TriggerAbilityFX(EAbilitySlot Slot)
{
    if (UM1AbilityBase* Ability = AbilityInstances.FindRef(Slot))
        Ability->OnServerConfirmed(GetOwnerCharacter());
}

bool UM1PlayerActorComponent::IsBasicAttackPendingStop() const
{
    UM1AbilityBase* Ability = AbilityInstances.FindRef(EAbilitySlot::BasicAttack);
    if (UM1Ability_BasicAttack* Attack = Cast<UM1Ability_BasicAttack>(Ability))
        return Attack->IsPendingStop();
    return false;
}

AM1Character* UM1PlayerActorComponent::GetOwnerCharacter() const
{
    return Cast<AM1Character>(GetOwner());
}

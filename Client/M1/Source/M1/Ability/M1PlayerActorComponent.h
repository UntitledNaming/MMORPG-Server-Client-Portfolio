// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Ability\M1AbilityTypes.h"
#include "M1PlayerActorComponent.generated.h"

class UM1AbilityBase;
class UM1CharacterAbilityDataAsset;
class AM1Character;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillCoolTimeStarted, EAbilitySlot, Slot, float, Duration);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class M1_API UM1PlayerActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    UM1PlayerActorComponent();

    void InitializeFromData(UM1CharacterAbilityDataAsset* Data);
    void ActivateAbility(EAbilitySlot Slot);
    void DeactivateAbility(EAbilitySlot Slot);
    void OnSkillResponse(EAbilitySlot Slot, bool bSuccess);
    void TriggerAbilityFX(EAbilitySlot Slot);
    bool IsBasicAttackPendingStop() const;

private:
    UPROPERTY()
    TMap<EAbilitySlot, UM1AbilityBase*> AbilityInstances;
    AM1Character* GetOwnerCharacter() const;

public:
    UPROPERTY(BlueprintAssignable)
    FOnSkillCoolTimeStarted OnCoolTimeStarted;
};

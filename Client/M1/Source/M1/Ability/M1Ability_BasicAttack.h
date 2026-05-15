// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ability/M1AbilityBase.h"
#include "M1Ability_BasicAttack.generated.h"

class AM1Character;

UCLASS(Blueprintable)
class M1_API UM1Ability_BasicAttack : public UM1AbilityBase
{
	GENERATED_BODY()

public:
    virtual void OnActivate(AM1Character* Owner) override;
    virtual void OnDeactivate(AM1Character* Owner) override;

    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    bool IsPendingStop() const { return bPendingStop; }

private:
    bool          bIsAttacking = false;
    bool          bPendingStop = false;
    AM1Character* CachedOwner  = nullptr;
};

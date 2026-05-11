// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ability/M1AbilityBase.h"
#include "M1Ability_BasicAttack.generated.h"

UCLASS(Blueprintable)
class M1_API UM1Ability_BasicAttack : public UM1AbilityBase
{
	GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
    float AttackInterval = 2.4f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
    float AttackRange = 150.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
    float AttackHalfAngle = 60.f;

    virtual void OnActivate(AM1Character* Owner) override;
    virtual void OnDeactivate(AM1Character* Owner) override;

private:
    void PerformAttack(AM1Character* Owner, class UM1NetworkManager* NetworkManager);

private:
    FTimerHandle AttackTimerHandle;
    bool         bIsAttacking = false;
};

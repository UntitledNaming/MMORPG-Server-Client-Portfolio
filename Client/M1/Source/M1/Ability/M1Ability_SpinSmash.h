// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ability/M1AbilityBase.h"
#include "M1Ability_SpinSmash.generated.h"

/**
 * 
 */
UCLASS()
class M1_API UM1Ability_SpinSmash : public UM1AbilityBase
{
	GENERATED_BODY()
public:
	UM1Ability_SpinSmash();

	virtual void OnActivate(AM1Character* Owner) override;

	// 서버로부터 응답 왔을 때
	virtual void OnServerConfirmed(AM1Character* Owner) override;

private:
	AM1Character* CachedOwner = nullptr;
};

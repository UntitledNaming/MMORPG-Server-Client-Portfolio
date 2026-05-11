// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Ability\M1AbilityTypes.h"
#include "Ability\M1AbilityBase.h"
#include "M1CharacterAbilityDataAsset.generated.h"


// 슬롯 하나의 설정 (클래스 + FX/사운드/애니 데이터)
USTRUCT(BlueprintType)
struct FAbilityEntry
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<UM1AbilityBase> AbilityClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FAbilityFXData FXData;
};

UCLASS(BlueprintType)
class M1_API UM1CharacterAbilityDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<EAbilitySlot, FAbilityEntry> AbilityEntries;
};

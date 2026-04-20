// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "M1GameInstance.generated.h"

/**
 * 
 */
UCLASS()
class M1_API UM1GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
    UM1GameInstance(const FObjectInitializer& ObjectInitializer);

public:
    virtual void Init() override;
    virtual void Shutdown() override;
};

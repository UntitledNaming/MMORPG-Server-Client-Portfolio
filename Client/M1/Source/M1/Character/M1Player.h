// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "M1Character.h"
#include "M1Player.generated.h"

UCLASS()
class M1_API AM1Player : public AM1Character
{
	GENERATED_BODY()

public:
    AM1Player();

public:
    virtual void ApplySpawnData(const FM1SpawnData& Data);
    virtual void ApplyStateData(EM1ActionType NewAction, uint8 NewInputMask);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    TObjectPtr<class USpringArmComponent> SpringArmComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    TObjectPtr<class UCameraComponent> CameraComponent;
	
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
    FString NickName;

    UPROPERTY(VisibleAnywhere)
    int32 MP = 0;
};

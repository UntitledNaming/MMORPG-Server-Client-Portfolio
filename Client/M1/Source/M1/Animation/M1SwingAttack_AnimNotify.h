// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "M1SwingAttack_AnimNotify.generated.h"

/**
 * 
 */
UCLASS()
class M1_API UM1SwingAttack_AnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
    uint8 SwingIndex = 0;

    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};

// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/M1InputDataAsset.h"
#include "M1/System/M1LogClass.h"

const UInputAction* UM1InputDataAsset::FindInputActionByTag(const FGameplayTag& InputTag) const
{
    for (const FM1InputAction& Action : InputActions)
    {
        if (Action.InputAction && Action.InputTag == InputTag)
            return Action.InputAction;
    }

    UE_LOG(LogM1, Error, TEXT("Can't find InputAction for InputTag [%s]"), *InputTag.ToString())

    return nullptr;
}
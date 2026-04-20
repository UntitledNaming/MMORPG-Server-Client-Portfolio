#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "M1InputDataAsset.generated.h"

class UInputAction;
class UInputMappingContext;

USTRUCT()
struct FM1InputAction
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly)
    FGameplayTag InputTag = FGameplayTag::EmptyTag;

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UInputAction> InputAction = nullptr;
};


UCLASS()
class M1_API UM1InputDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
    const UInputAction* FindInputActionByTag(const FGameplayTag& InputTag) const;

public:
    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<class UInputMappingContext> InputMappingContext;

    UPROPERTY(EditDefaultsOnly)
    TArray<FM1InputAction> InputActions;
};

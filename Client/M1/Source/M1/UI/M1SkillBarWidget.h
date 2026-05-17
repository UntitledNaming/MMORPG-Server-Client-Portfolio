#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Ability\M1AbilityTypes.h"
#include "M1SkillBarWidget.generated.h"

class UM1PlayerActorComponent;
class UM1SkillSlotWidget;

UCLASS()
class M1_API UM1SkillBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    void BindToComponent(UM1PlayerActorComponent* AbilityComp);

private:
    UFUNCTION()
    void OnCoolTimeStarted(EAbilitySlot SkillSlot, float Duration);

    UPROPERTY(meta = (BindWidget)) UM1SkillSlotWidget* SkillSlot_1;
    UPROPERTY(meta = (BindWidget)) UM1SkillSlotWidget* SkillSlot_2;
    UPROPERTY(meta = (BindWidget)) UM1SkillSlotWidget* SkillSlot_3;
    UPROPERTY(meta = (BindWidget)) UM1SkillSlotWidget* SkillSlot_4;
};

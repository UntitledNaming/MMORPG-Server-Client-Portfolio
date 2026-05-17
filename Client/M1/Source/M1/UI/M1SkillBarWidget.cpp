#include "UI/M1SkillBarWidget.h"
#include "UI\M1SkillSlotWidget.h"
#include "Ability\M1PlayerActorComponent.h"


void UM1SkillBarWidget::BindToComponent(UM1PlayerActorComponent* AbilityComp)
{
    if (!AbilityComp) return;
    AbilityComp->OnCoolTimeStarted.AddDynamic(this, &UM1SkillBarWidget::OnCoolTimeStarted);
}

void UM1SkillBarWidget::OnCoolTimeStarted(EAbilitySlot SkillSlot, float Duration)
{
    UM1SkillSlotWidget* Slots[] = { SkillSlot_1, SkillSlot_2, SkillSlot_3, SkillSlot_4 };
    int32 Index = static_cast<int32>(SkillSlot) - 1;
    if (Slots[Index])
        Slots[Index]->StartCooldown(Duration);
}
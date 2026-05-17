// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/M1SkillSlotWidget.h"

void UM1SkillSlotWidget::NativeConstruct()
{
    Super::NativeConstruct();

    ClearCooldown();
}

void UM1SkillSlotWidget::StartCooldown(float CooldownSeconds)
{
    if (CooldownSeconds <= 0.f)
    {
        ClearCooldown();
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
        return;

    bCoolTime = true;
    CoolTimeDuration = CooldownSeconds;
    CoolTimeEndTime = World->GetTimeSeconds() + CooldownSeconds;

    if (Image_CooldownOverlay)
    {
        Image_CooldownOverlay->SetVisibility(ESlateVisibility::HitTestInvisible);
    }

    if (Text_CoolTime)
    {
        Text_CoolTime->SetVisibility(ESlateVisibility::HitTestInvisible);
    }

    UpdateCooldownUI(CooldownSeconds);
}

void UM1SkillSlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!bCoolTime)
        return;

    UWorld* World = GetWorld();
    if (!World)
        return;

    const float RemainSeconds = static_cast<float>(CoolTimeEndTime - World->GetTimeSeconds());

    if (RemainSeconds <= 0.f)
    {
        ClearCooldown();
        return;
    }

    UpdateCooldownUI(RemainSeconds);
}

void UM1SkillSlotWidget::UpdateCooldownUI(float RemainSeconds)
{
    if (Text_CoolTime)
    {
        const int32 DisplaySeconds = FMath::CeilToInt(RemainSeconds);
        Text_CoolTime->SetText(FText::AsNumber(DisplaySeconds));
    }
}

void UM1SkillSlotWidget::ClearCooldown()
{
    bCoolTime = false;
    CoolTimeDuration = 0.f;
    CoolTimeEndTime = 0.0;

    if (Image_CooldownOverlay)
    {
        Image_CooldownOverlay->SetVisibility(ESlateVisibility::Hidden);
    }

    if (Text_CoolTime)
    {
        Text_CoolTime->SetVisibility(ESlateVisibility::Hidden);
        Text_CoolTime->SetText(FText::GetEmpty());
    }
}
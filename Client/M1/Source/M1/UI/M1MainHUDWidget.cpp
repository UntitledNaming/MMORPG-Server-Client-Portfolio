// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/M1MainHUDWidget.h"
#include "Character/M1LocalPlayer.h"
#include "M1SkillBarWidget.h"
#include "M1InventoryPanelWidget.h"
#include "M1EquipmentPanelWidget.h"

void UM1MainHUDWidget::BindToPlayer(AM1LocalPlayer* Player)
{
    if (!Player) return;

    OwningPlayer = Player;

    // 델리게이트 바인딩
    Player->OnHealthChanged.AddDynamic(this, &UM1MainHUDWidget::OnHealthChanged);
    Player->OnManaChanged.AddDynamic(this, &UM1MainHUDWidget::OnManaChanged);
    Player->OnExpChanged.AddDynamic(this, &UM1MainHUDWidget::OnExpChanged);
    Player->OnLevelUpdate.AddDynamic(this, &UM1MainHUDWidget::OnLevelUpdate);

    // 초기값 UI에 반영
    Player->OnHealthChanged.Broadcast(Player->GetCurrentHealth(), Player->GetMaxHealth());
    Player->OnManaChanged.Broadcast(Player->GetCurrentMana(), Player->GetMaxMana());
    Player->OnExpChanged.Broadcast(Player->GetCurrentExp(), Player->GetRequiredExp());
    Player->OnLevelUpdate.Broadcast(Player->GetLevel());

    if (SkillBar)
        SkillBar->BindToComponent(Player->GetAbilityComponent());
}

void UM1MainHUDWidget::OnHealthChanged(int32 CurrentHealth, int32 MaxHealth)
{
    UpdateHealthBar(CurrentHealth, MaxHealth);
    UpdateHealthText(CurrentHealth, MaxHealth);
}

void UM1MainHUDWidget::OnManaChanged(int32 CurrentMana, int32 MaxMana)
{
    UpdateManaBar(CurrentMana, MaxMana);
    UpdateManaText(CurrentMana, MaxMana);
}

void UM1MainHUDWidget::OnExpChanged(float CurrentExp, float RequiredExp)
{
    UpdateExpBar(CurrentExp, RequiredExp);
}

void UM1MainHUDWidget::OnLevelUpdate(int32 Level)
{
    UpdateLevel(Level);
}

void UM1MainHUDWidget::ToggleInventoryPanel()
{
    if (!InventoryPanel)
        return;

    const bool bVisible = InventoryPanel->GetVisibility() == ESlateVisibility::Visible;
    InventoryPanel->SetVisibility(bVisible ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
}

void UM1MainHUDWidget::ToggleEquipmentPanel()
{
    if (!EquipmentPanel)
        return;

    const bool bVisible = EquipmentPanel->GetVisibility() == ESlateVisibility::Visible;
    EquipmentPanel->SetVisibility(bVisible ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
}

bool UM1MainHUDWidget::IsAnyItemPanelOpen() const
{
    const bool bInventoryOpen =
        InventoryPanel && InventoryPanel->GetVisibility() == ESlateVisibility::Visible;

    const bool bEquipmentOpen =
        EquipmentPanel && EquipmentPanel->GetVisibility() == ESlateVisibility::Visible;

    return bInventoryOpen || bEquipmentOpen;
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/M1MainHUDWidget.h"
#include "Character/M1LocalPlayer.h"


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
}

void UM1MainHUDWidget::OnHealthChanged(int32 CurrentHealth, int32 MaxHealth)
{
    if (CurrentHealth > 0)
    {
        float Ratio = 0.0f;

        Ratio = static_cast<float>(CurrentHealth) / static_cast<float>(MaxHealth);
        Ratio = FMath::Clamp(Ratio, 0.0f, 1.0f);
        Ratio = FMath::Max(Ratio, 0.02f);
    }

    UpdateHealthBar(CurrentHealth, MaxHealth);
}

void UM1MainHUDWidget::OnManaChanged(int32 CurrentMana, int32 MaxMana)
{
    UpdateManaBar(CurrentMana, MaxMana);
}

void UM1MainHUDWidget::OnExpChanged(float CurrentExp, float RequiredExp)
{
    UpdateExpBar(CurrentExp, RequiredExp);
}

void UM1MainHUDWidget::OnLevelUpdate(int32 Level)
{
    UpdateLevel(Level);
}
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "M1MainHUDWidget.generated.h"

class UM1SkillBarWidget;

UCLASS()
class M1_API UM1MainHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    // 블루프린트에서 구현할 함수들 (BlueprintImplementableEvent)
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void UpdateHealthBar(int32 CurrentHealth, int32 MaxHealth);

    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void UpdateManaBar(int32 CurrentMana, int32 MaxMana);

    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void UpdateExpBar(float CurrentExp, float RequiredExp);

    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void UpdateLevel(int32 Level);

    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void UpdateHealthText(int32 CurrentHealth, int32 MaxHealth);

    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void UpdateManaText(int32 CurrentMana, int32 MaxMana);


    // C++에서 호출할 바인딩 함수
    UFUNCTION(BlueprintCallable, Category = "UI")
    void BindToPlayer(class AM1LocalPlayer* Player);

protected:
    UPROPERTY()
    class AM1LocalPlayer* OwningPlayer;

    UFUNCTION()
    void OnHealthChanged(int32 CurrentHealth, int32 MaxHealth);

    UFUNCTION()
    void OnManaChanged(int32 CurrentMana, int32 MaxMana);

    UFUNCTION()
    void OnExpChanged(float CurrentExp, float RequiredExp);

    UFUNCTION()
    void OnLevelUpdate(int32 Level);

protected:
    UPROPERTY(meta = (BindWidget))
    UM1SkillBarWidget* SkillBar;
};

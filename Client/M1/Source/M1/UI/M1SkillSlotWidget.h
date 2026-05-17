#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "M1SkillSlotWidget.generated.h"

UCLASS()
class M1_API UM1SkillSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
    void StartCooldown(float CooldownSeconds);
    void ClearCooldown();

private:
    void UpdateCooldownUI(float RemainSeconds);

private:
    UPROPERTY(meta = (BindWidget))
    UImage* Image_CooldownOverlay = nullptr;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_CoolTime = nullptr;

private:
    bool   bCoolTime = false;

    float  CoolTimeDuration = 0.f;
    double CoolTimeEndTime = 0.0;
};

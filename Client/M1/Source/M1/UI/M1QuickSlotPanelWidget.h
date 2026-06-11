#pragma once

#include "CoreMinimal.h"
#include "ContentsEnum.h"
#include "Blueprint/UserWidget.h"
#include "M1QuickSlotPanelWidget.generated.h"

class UM1ItemManager;
class UM1ItemSlotWidget;

UCLASS()
class M1_API UM1QuickSlotPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnQuickSlotChanged();

private:
    UPROPERTY(meta = (BindWidget))
    UM1ItemSlotWidget* Slot_0 = nullptr;

    UPROPERTY(meta = (BindWidget))
    UM1ItemSlotWidget* Slot_1 = nullptr;

    TArray<UM1ItemSlotWidget*> SlotWidgets;

    UPROPERTY()
    UM1ItemManager* ItemManager = nullptr;
};

#pragma once

#include "CoreMinimal.h"
#include "ContentsEnum.h"
#include "Blueprint/UserWidget.h"
#include "M1EquipmentPanelWidget.generated.h"

class UM1ItemManager;
class UM1ItemSlotWidget;

UCLASS()
class M1_API UM1EquipmentPanelWidget : public UUserWidget
{
	GENERATED_BODY()

private:
    UPROPERTY(meta = (BindWidget))
    UM1ItemSlotWidget* EquipSlot_Helmet = nullptr;

    UPROPERTY(meta = (BindWidget))
    UM1ItemSlotWidget* EquipSlot_Chest = nullptr;

    UPROPERTY(meta = (BindWidget))
    UM1ItemSlotWidget* EquipSlot_Pants = nullptr;

    UPROPERTY(meta = (BindWidget))
    UM1ItemSlotWidget* EquipSlot_Boots = nullptr;

    UPROPERTY(meta = (BindWidget))
    UM1ItemSlotWidget* EquipSlot_Weapon = nullptr;

    TArray<UM1ItemSlotWidget*> SlotWidgets;

    UPROPERTY()
    UM1ItemManager* ItemManager = nullptr;

public:
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnEquipmentChanged();
};

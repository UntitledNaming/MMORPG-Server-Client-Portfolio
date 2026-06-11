#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ContentsEnum.h"
#include "M1InventoryPanelWidget.generated.h"

class UM1ItemManager;
class UM1ItemSlotWidget;
class UUniformGridPanel;

UCLASS()
class M1_API UM1InventoryPanelWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnInventoryChanged();

private:
    UPROPERTY(meta = (BindWidget))
    UUniformGridPanel* Grid_Slots = nullptr;

    TArray<UM1ItemSlotWidget*> SlotWidgets;

    UPROPERTY()
    UM1ItemManager* ItemManager = nullptr;
};

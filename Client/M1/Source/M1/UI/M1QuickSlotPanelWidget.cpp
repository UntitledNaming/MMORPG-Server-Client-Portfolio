#include "UI/M1QuickSlotPanelWidget.h"
#include "UI/M1ItemSlotWidget.h"
#include "System/M1ItemManager.h"

void UM1QuickSlotPanelWidget::NativeConstruct()
{
    Super::NativeConstruct();

    ItemManager = GetGameInstance()->GetSubsystem<UM1ItemManager>();

    SlotWidgets.Add(Slot_0);
    SlotWidgets.Add(Slot_1);

    for (int32 i = 0; i < SlotWidgets.Num(); i++)
    {
        if (SlotWidgets[i])
        {
            SlotWidgets[i]->InitSlot(SLOT_TYPE::QUICKSLOT, static_cast<int16>(i));

        }
    }

    ItemManager->OnQuickSlotChanged.AddDynamic(this, &UM1QuickSlotPanelWidget::OnQuickSlotChanged);
}

void UM1QuickSlotPanelWidget::OnQuickSlotChanged()
{
    for (int i = 0; i < SlotWidgets.Num(); i++)
    {
        if (SlotWidgets[i])
            SlotWidgets[i]->RefreshSlot();
    }

}

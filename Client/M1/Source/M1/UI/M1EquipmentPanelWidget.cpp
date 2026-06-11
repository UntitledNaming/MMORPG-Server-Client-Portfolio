#include "UI/M1EquipmentPanelWidget.h"
#include "UI/M1ItemSlotWidget.h"
#include "System/M1ItemManager.h"

void UM1EquipmentPanelWidget::NativeConstruct()
{
    Super::NativeConstruct();

    ItemManager = GetGameInstance()->GetSubsystem<UM1ItemManager>();

    // 배열에 슬롯 위젯 저장
    SlotWidgets.Add(nullptr);
    SlotWidgets.Add(EquipSlot_Helmet);
    SlotWidgets.Add(EquipSlot_Chest);
    SlotWidgets.Add(EquipSlot_Pants);
    SlotWidgets.Add(EquipSlot_Boots);
    SlotWidgets.Add(EquipSlot_Weapon);

    for (int32 i = 1; i < SlotWidgets.Num(); i++)
    {
        if (SlotWidgets[i])
            SlotWidgets[i]->InitSlot(SLOT_TYPE::EQUIPMENT, static_cast<int16>(i));
    }

    ItemManager->OnEquipmentChanged.AddDynamic(this, &UM1EquipmentPanelWidget::OnEquipmentChanged);
}

void UM1EquipmentPanelWidget::OnEquipmentChanged()
{
    for (int i = 0; i < SlotWidgets.Num(); i++)
    {
        if (SlotWidgets[i])
            SlotWidgets[i]->RefreshSlot();
    }
}
#include "UI/M1InventoryPanelWidget.h"
#include "UI/M1ItemSlotWidget.h"
#include "System/M1ItemManager.h"
#include "Components/UniformGridPanel.h"

void UM1InventoryPanelWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 아이템 매니저 가져오기
    ItemManager = GetGameInstance()->GetSubsystem<UM1ItemManager>();

    // BP에 설정한 WBP_ItemSlot을 하나씩 순회하면서 InitSlot 호출해서 초기화 및 InventoryPanel 배열에 저장함.
    for (int32 i = 0; i < Grid_Slots->GetChildrenCount(); i++)
    {
        UM1ItemSlotWidget* ItemSlotWidget = Cast<UM1ItemSlotWidget>(Grid_Slots->GetChildAt(i));
        if (ItemSlotWidget)
        {
            ItemSlotWidget->InitSlot(SLOT_TYPE::INVENTORY, static_cast<int16>(i));
            SlotWidgets.Add(ItemSlotWidget);
        }
    }


    // ItemManager 델리게이트에 콜백 함수인 OnInventoryChanged 등록
    ItemManager->OnInventoryChanged.AddDynamic(this, &UM1InventoryPanelWidget::OnInventoryChanged);
}

void UM1InventoryPanelWidget::OnInventoryChanged()
{
    // 슬롯 위젯 순회하면서 슬롯 갱신
    for (int i = 0; i < SlotWidgets.Num(); i++)
    {
        if (SlotWidgets[i])
            SlotWidgets[i]->RefreshSlot();
    }
}


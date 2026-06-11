#include "System/M1ItemManager.h"
#include "NetPacketHeader.h"
#include "Network/M1NetworkManager.h"
#include "Network\ClientCore\CMessage.h"
#include "Network\M1NetworkManager.h"
#include "UI\M1DraggedItemWidget.h"
#include "System\M1SpawnManager.h"

void UM1ItemManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency<UM1NetworkManager>();
	Super::Initialize(Collection);

	NetworkManager = GetGameInstance()->GetSubsystem<UM1NetworkManager>();

	Inventory.SetNum(UserInventory::INVENTORY_SLOT_MAX); // 40개
	Equipment.SetNum((uint8)EQUIP_SLOT::MAX);            
	QuickSlot.SetNum(UserQuickSlot::QUICK_SLOT_MAX);     // 2개

	FM1ItemTable::Init();
}

void UM1ItemManager::Deinitialize()
{
	Super::Deinitialize();

	FM1ItemTable::Destroy();
}

void UM1ItemManager::OnPickupEquipmentItem(const PickUpEquipResult& Result)
{
	bPendingSlotRequest = false;

	// 슬롯 위치 참조 및 초기화
	FItemSlotData& Slot = Inventory[Result.slotIndex];
	Slot.ItemID = Result.itemID;
	Slot.Count = Result.count;
	Slot.RandomStats.Reset();

	// 결과 랜덤 스탯을 해당 슬롯에 랜덤 스탯 배열에 넣기
	for (int i = 0; i< Result.randomStatCount; i++)
	{
		FRandomStat stat;
		stat.RandomStatType = Result.randomStatResult[i].randomStatType;
		stat.RandomStatValue = Result.randomStatResult[i].randomStatValue;
		Slot.RandomStats.Add(stat);
	}

	OnInventoryChanged.Broadcast();
}

void UM1ItemManager::OnPickupConsumableItem(const PickUpConsumableResult& Result)
{
	bPendingSlotRequest = false;

	for (int i = 0; i < Result.updateSlotCount; i++)
	{
		int16 idx = Result.consumableResult[i].slotIndex;
		Inventory[idx].ItemID = Result.itemID;
		Inventory[idx].Count = Result.consumableResult[i].newItemCount;
	}

	OnInventoryChanged.Broadcast();
}

void UM1ItemManager::OnUseConsumableResult(bool bSuccess, USE_CONSUMABLE_ITEM_RESULT& Result)
{
	bPendingSlotRequest = false;
	if (!bSuccess)
		return;

	// 슬롯 업데이트 전에 ItemID 먼저 읽기
	ITEM_ID UsedItemID = 0;

	switch (Result.slotType)
	{
	case SLOT_TYPE::INVENTORY:
		UsedItemID = Inventory[Result.slotIndex].ItemID;
		Inventory[Result.slotIndex].Count = Result.newItenCount;
		if (Result.newItenCount == 0)
			Inventory[Result.slotIndex].ItemID = 0;  // 아이템 슬롯 비우기
		OnInventoryChanged.Broadcast();
		break;

	case SLOT_TYPE::QUICKSLOT:
		UsedItemID = QuickSlot[Result.slotIndex].ItemID;
		QuickSlot[Result.slotIndex].Count = Result.newItenCount;
		if (Result.newItenCount == 0)
			QuickSlot[Result.slotIndex].ItemID = 0;

		OnQuickSlotChanged.Broadcast();
		break;
	}

	// HP/MP 회복 적용
	const ItemData* Data = FM1ItemTable::GetItemData(UsedItemID);
	if (Data && (Data->recoverHP > 0 || Data->recoverMP > 0))
	{
		AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();
		if (SpawnManager)
			SpawnManager->ApplyConsumableRecovery(Data->recoverHP, Data->recoverMP);
	}
}

void UM1ItemManager::OnEquipItemResult(bool bSuccess, EQUIP_ITEM_RESULT& Result)
{
	bPendingSlotRequest = false;

	if (!bSuccess)
		return;

	FItemSlotData* InvSlot = nullptr;
	FItemSlotData* EquipSlot = nullptr;

	for (uint8 i = 0; i < Result.updateSlotCount; ++i)
	{
		const UPDATE_SLOT& s = Result.resultSlot[i];

		// 결과 반영할 슬롯의 주소 얻기
		if (s.slotType == SLOT_TYPE::INVENTORY)
			InvSlot = &Inventory[s.slotIndex];
		else if (s.slotType == SLOT_TYPE::EQUIPMENT)
			EquipSlot = &Equipment[s.slotIndex];
	}

	if (InvSlot && EquipSlot)
	{
		FItemSlotData Temp = *EquipSlot;
		*EquipSlot = *InvSlot;
		*InvSlot = Temp;
	}

	OnInventoryChanged.Broadcast();
	OnEquipmentChanged.Broadcast();
}

void UM1ItemManager::OnUnequipItemResult(bool bSuccess, UNEQUIP_ITEM_RESULT& Result)
{
	bPendingSlotRequest = false;

	if (!bSuccess)
		return;

	// 눌렀을 때의 슬롯 타입이 장비가 아니면 리턴
	if (!(PendingRequest.FromSlotType == SLOT_TYPE::EQUIPMENT))
		return;

	// 장비에 있는 기존 데이터 인벤토리로 복사
	Inventory[Result.inventorySlotIdx] = Equipment[PendingRequest.FromSlotIndex];

	// 장비 탭 비우기
	Equipment[PendingRequest.FromSlotIndex].ItemID = 0;

	OnInventoryChanged.Broadcast();
	OnEquipmentChanged.Broadcast();
}

void UM1ItemManager::OnDeleteItemResult(bool bSuccess)
{
	bPendingSlotRequest = false;

	if (!bSuccess)
		return;

	switch (PendingRequest.FromSlotType)
	{
	case SLOT_TYPE::INVENTORY:
		Inventory[PendingRequest.FromSlotIndex].ItemID = 0;
		OnInventoryChanged.Broadcast();
		break;

	case SLOT_TYPE::EQUIPMENT:
		Equipment[PendingRequest.FromSlotIndex].ItemID = 0;
		OnEquipmentChanged.Broadcast();
		break;

	case SLOT_TYPE::QUICKSLOT:
		QuickSlot[PendingRequest.FromSlotIndex].ItemID = 0;
		OnQuickSlotChanged.Broadcast();
		break;
	}

}

void UM1ItemManager::OnSwapSlotResult(bool bSuccess)
{
	bPendingSlotRequest = false;

	if (!bSuccess)
		return;


	FItemSlotData* FromData = nullptr;
	FItemSlotData* ToData = nullptr;

	// From 데이터 저장
	switch (PendingRequest.FromSlotType)
	{
	case SLOT_TYPE::EQUIPMENT:
		FromData = &Equipment[PendingRequest.FromSlotIndex];
		break;

	case SLOT_TYPE::INVENTORY:
		FromData = &Inventory[PendingRequest.FromSlotIndex];
		break;


	case SLOT_TYPE::QUICKSLOT:
		FromData = &QuickSlot[PendingRequest.FromSlotIndex];
		break;
	}

	// To 데이터 저장
	switch (PendingRequest.ToSlotType)
	{
	case SLOT_TYPE::EQUIPMENT:
		ToData = &Equipment[PendingRequest.ToSlotIndex];
		break;

	case SLOT_TYPE::INVENTORY:
		ToData = &Inventory[PendingRequest.ToSlotIndex];
		break;

	case SLOT_TYPE::QUICKSLOT:
		ToData = &QuickSlot[PendingRequest.ToSlotIndex];
		break;
	}

	// Swap
	if (FromData && ToData)
	{
		FItemSlotData Temp = *ToData;

		*ToData = *FromData;
		*FromData = Temp;
	}

	OnInventoryChanged.Broadcast();
	OnEquipmentChanged.Broadcast();
	OnQuickSlotChanged.Broadcast();
}

// ── UI가 호출  ────────────────────────
void UM1ItemManager::TrySendUseItem(uint8 SlotType, int16 SlotIndex)
{
	if (bPendingSlotRequest == true)
		return;

	CMessage* msg = CMessage::Alloc();
	*msg << (uint16)FieldProtocol::PACKET_CS_USE_ITEM << SlotType << SlotIndex;
	NetworkManager->SendPacket(msg, static_cast<uint8>(ERouteType::GROUP), ServiceID::NONE_SERVICE);
	PendingRequest.RequestType = ESlotRequestType::Use;
	PendingRequest.FromSlotIndex = SlotIndex;
	PendingRequest.FromSlotType = static_cast<SLOT_TYPE>(SlotType);
	bPendingSlotRequest = true;
	CMessage::Free(msg);
}

void UM1ItemManager::TrySendDeleteItem(uint8 SlotType, int16 SlotIndex)
{
	if (bPendingSlotRequest == true)
		return;

	CMessage* msg = CMessage::Alloc();
	*msg << (uint16)FieldProtocol::PACKET_CS_DELETE_ITEM << SlotType << SlotIndex;
	NetworkManager->SendPacket(msg, static_cast<uint8>(ERouteType::GROUP), ServiceID::NONE_SERVICE);
	PendingRequest.RequestType = ESlotRequestType::Delete;
	PendingRequest.FromSlotIndex = SlotIndex;
	PendingRequest.FromSlotType = static_cast<SLOT_TYPE>(SlotType);

	bPendingSlotRequest = true;
	CMessage::Free(msg);
}

void UM1ItemManager::TrySendSwapSlot(uint8 FromType, int16 FromIdx, uint8 ToType, int16 ToIdx)
{
	if (bPendingSlotRequest == true)
		return;

	CMessage* msg = CMessage::Alloc();
	*msg << (uint16)FieldProtocol::PACKET_CS_SWAP_SLOT << FromType << FromIdx << ToType << ToIdx;
	NetworkManager->SendPacket(msg, static_cast<uint8>(ERouteType::GROUP), ServiceID::NONE_SERVICE);
	PendingRequest.RequestType = ESlotRequestType::Swap;
	PendingRequest.FromSlotIndex = FromIdx;
	PendingRequest.FromSlotType = static_cast<SLOT_TYPE>(FromType);
	PendingRequest.ToSlotIndex = ToIdx;
	PendingRequest.ToSlotType = static_cast<SLOT_TYPE>(ToType);

	bPendingSlotRequest = true;
	CMessage::Free(msg);
}

void UM1ItemManager::StartDrag(SLOT_TYPE Type, int16 Index, UM1DraggedItemWidget* Widget)
{
	DragStartSourceType = Type;
	DragStartSourceIndex = Index;
	ActiveDragWidget = Widget;

	bIsDragging = true;
}

void UM1ItemManager::EndDrag()
{
	bIsDragging = false;
	DragStartSourceType = SLOT_TYPE::NONE;
	DragStartSourceIndex = -1;
	DragHoverType = SLOT_TYPE::NONE;
	DragHoverIndex = -1;
	if (ActiveDragWidget)
	{
		ActiveDragWidget->RemoveFromParent();
		ActiveDragWidget = nullptr;
	}
}

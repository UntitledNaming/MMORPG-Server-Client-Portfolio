#include "System/M1ItemManager.h"
#include "NetPacketHeader.h"
#include "Network/M1NetworkManager.h"
#include "Network\ClientCore\CMessage.h"
#include "Network\M1NetworkManager.h"
#include "UI\M1DraggedItemWidget.h"
#include "UI\M1ItemTooltipWidget.h"
#include "System\M1SpawnManager.h"
#include "Character\M1LocalPlayer.h"

void UM1ItemManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency<UM1NetworkManager>();
	Super::Initialize(Collection);

	NetworkManager = GetGameInstance()->GetSubsystem<UM1NetworkManager>();

	Inventory.SetNum(UserInventory::INVENTORY_SLOT_MAX); // 40개
	Equipment.SetNum((uint8)EQUIP_SLOT::MAX);            
	QuickSlot.SetNum(UserQuickSlot::QUICK_SLOT_MAX);     // 2개

	for (int i = 0; i < UserInventory::INVENTORY_SLOT_MAX; i++)
	{
		Inventory[i].ItemID = 0;
		Inventory[i].Count = 0;
		Inventory[i].RandomStats = {};
	}

	for (int i = 1; i < (uint8)EQUIP_SLOT::MAX; i++)
	{
		Equipment[i].ItemID = 0;
		Equipment[i].Count = 0;
		Equipment[i].RandomStats = {};
	}

	for (int i = 0; i < UserQuickSlot::QUICK_SLOT_MAX; i++)
	{
		QuickSlot[i].ItemID = 0;
		QuickSlot[i].Count = 0;
		QuickSlot[i].RandomStats = {};
	}


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
	{
		switch (PendingRequest.FromSlotType)
		{
		case SLOT_TYPE::INVENTORY:  OnInventoryChanged.Broadcast();  break;
		case SLOT_TYPE::QUICKSLOT:  OnQuickSlotChanged.Broadcast();  break;
		}
		return;
	}

	// 슬롯 업데이트 전에 ItemID 먼저 읽기
	ITEM_ID UsedItemID = 0;

	switch (Result.slotType)
	{
	case SLOT_TYPE::INVENTORY:
		UsedItemID = Inventory[Result.slotIndex].ItemID;
		Inventory[Result.slotIndex].Count = Result.newItenCount;
		if (Result.newItenCount == 0)
			Inventory[Result.slotIndex].ItemID = 0;  // 아이템 슬롯 비우기
		break;

	case SLOT_TYPE::QUICKSLOT:
		UsedItemID = QuickSlot[Result.slotIndex].ItemID;
		QuickSlot[Result.slotIndex].Count = Result.newItenCount;
		if (Result.newItenCount == 0)
			QuickSlot[Result.slotIndex].ItemID = 0;
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

	// "사용한 슬롯"에 쿨타임을 건다.
	StartSlotCoolTime(Result.slotType, Result.slotIndex, UsedItemID);

	switch (Result.slotType)
	{
	case SLOT_TYPE::INVENTORY:
		OnInventoryChanged.Broadcast();
		break;

	case SLOT_TYPE::QUICKSLOT:
		OnQuickSlotChanged.Broadcast();
		break;
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


	OnInventoryChanged.Broadcast();
	OnEquipmentChanged.Broadcast();
	RefreshEquipStat();
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
	RefreshEquipStat();
}

void UM1ItemManager::OnDeleteItemResult(bool bSuccess)
{
	bPendingSlotRequest = false;

	// 실패해도 Broadcast →RefreshSlot에서 이미지 복원
	if (!bSuccess)
	{
		switch (PendingRequest.FromSlotType)
		{
		case SLOT_TYPE::INVENTORY:  OnInventoryChanged.Broadcast();  break;
		case SLOT_TYPE::EQUIPMENT:  OnEquipmentChanged.Broadcast();  break;
		case SLOT_TYPE::QUICKSLOT:  OnQuickSlotChanged.Broadcast();  break;
		}
		return;
	}

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

	// 실패해도 Broadcast →RefreshSlot에서 이미지 복원
	if (!bSuccess)
	{
		OnInventoryChanged.Broadcast();
		OnEquipmentChanged.Broadcast();
		OnQuickSlotChanged.Broadcast();
		return;
	}


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

	if (IsSlotOnCoolTime(SlotType, SlotIndex))
		return;

	CMessage* msg = CMessage::Alloc();
	msg->Clear(1);
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

	if (IsSlotOnCoolTime(SlotType, SlotIndex))
		return;

	CMessage* msg = CMessage::Alloc();
	msg->Clear(1);
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

	if (IsSlotOnCoolTime(FromType, FromIdx))
		return;

	if (IsSlotOnCoolTime(ToType, ToIdx))
		return;


	CMessage* msg = CMessage::Alloc();
	msg->Clear(1);
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
	if (IsSlotOnCoolTime(static_cast<uint8>(Type), Index))
		return;

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

void UM1ItemManager::ShowTooltip(SLOT_TYPE Type, int16 Index, TSubclassOf<UM1ItemTooltipWidget> WidgetClass)
{
	if (!WidgetClass)
		return;

	const FItemSlotData* SlotData = nullptr;
	switch (Type)
	{
	case SLOT_TYPE::INVENTORY:  SlotData = &Inventory[Index];  break;
	case SLOT_TYPE::EQUIPMENT:  SlotData = &Equipment[Index];  break;
	case SLOT_TYPE::QUICKSLOT:  SlotData = &QuickSlot[Index];  break;
	}

	if (!SlotData || SlotData->ItemID == 0)
		return;

	// 최초 1회 생성
	if (!ActiveTooltipWidget)
	{
		UWorld* World = GetGameInstance()->GetWorld();
		APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
		if (!PC)
			return;

		ActiveTooltipWidget = CreateWidget<UM1ItemTooltipWidget>(PC, WidgetClass);
		if (!ActiveTooltipWidget)
			return;

		ActiveTooltipWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		ActiveTooltipWidget->AddToViewport(50);
	}

	ActiveTooltipWidget->SetItemData(SlotData->ItemID, SlotData->RandomStats);
	ActiveTooltipWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UM1ItemManager::HideTooltip()
{
	if (ActiveTooltipWidget)
		ActiveTooltipWidget->SetVisibility(ESlateVisibility::Hidden);
}

void  UM1ItemManager::ParseAndInitFromSpawn(CMessage* pMessage)
{
	// ── 인벤토리 ────────────────────────────────────
	uint8 invCount;
	*pMessage >> invCount;

	for (uint8 i = 0; i < invCount; ++i)
	{
		int16   slotIndex;
		ITEM_ID itemID;
		uint16  itemCount;
		uint8   randomStatCount;

		*pMessage >> slotIndex >> itemID >> itemCount >> randomStatCount;

		Inventory[slotIndex].ItemID = itemID;
		Inventory[slotIndex].Count = itemCount;
		Inventory[slotIndex].RandomStats.Reset();

		for (uint8 j = 0; j < randomStatCount; ++j)
		{
			uint8 statType; uint16 statValue;
			*pMessage >> statType >> statValue;
			Inventory[slotIndex].RandomStats.Add({ static_cast<RANDOM_STAT_TYPE>(statType), statValue });
		}
	}


	// ── 장비 ────────────────────────────────────────
	uint8 equipCount;
	*pMessage >> equipCount;

	for (uint8 i = 0; i < equipCount; ++i)
	{
		uint8   equipSlot;
		ITEM_ID itemID;
		uint8   randomStatCount;

		*pMessage >> equipSlot >> itemID >> randomStatCount;

		Equipment[equipSlot].ItemID = itemID;
		Equipment[equipSlot].Count = 1;
		Equipment[equipSlot].RandomStats.Reset();

		for (uint8 j = 0; j < randomStatCount; ++j)
		{
			uint8 statType; uint16 statValue;
			*pMessage >> statType >> statValue;
			Equipment[equipSlot].RandomStats.Add({ static_cast<RANDOM_STAT_TYPE>(statType), statValue });
		}
	}

	// ── 퀵슬롯 ──────────────────────────────────────
	uint8 quickCount;
	*pMessage >> quickCount;

	for (uint8 i = 0; i < quickCount && i < (uint8)QuickSlot.Num(); ++i)
	{
		ITEM_ID itemID; uint16 itemCount;
		*pMessage >> itemID >> itemCount;
		QuickSlot[i].ItemID = itemID;
		QuickSlot[i].Count = itemCount;
	}

	// ── 장비 스탯 반영 (장착 아이템 →EquipStat) ─────
	RefreshEquipStat();

	OnInventoryChanged.Broadcast();
	OnEquipmentChanged.Broadcast();
	OnQuickSlotChanged.Broadcast();
}

void UM1ItemManager::RefreshEquipStat()
{
	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();
	if (!SpawnManager) 
		return;

	AM1LocalPlayer* Player = SpawnManager->GetLocalPlayer();
	if (!Player) 
		return;


	Player->SetEquipStat(CalculateEquipStat());

}

FM1CharacterStat UM1ItemManager::CalculateEquipStat()
{
	FM1CharacterStat Total = {};

	for (int32 i = (int32)EQUIP_SLOT::HELMET; i < (int32)EQUIP_SLOT::MAX; ++i)
	{
		const FItemSlotData& Slot = Equipment[i];

		// 장착 아이템 없으면 다음 슬롯
		if (Slot.ItemID == 0) continue;

		// 있으면 아이템 테이블에서 찾기
		const ItemData* Data = FM1ItemTable::GetItemData(Slot.ItemID);
		if (!Data) continue;

		// 기본 스탯 더해주기
		Total.ATK += Data->baseStat.atk;
		Total.DEF += Data->baseStat.def;
		Total.MaxHP += Data->baseStat.maxHP;
		Total.MaxMP += Data->baseStat.maxMP;
		Total.HPRegenPerSec += Data->baseStat.hpRegenPerSec;
		Total.MPRegenPerSec += Data->baseStat.mpRegenPerSec;

		// 랜덤 스탯 더하기
		for (const FRandomStat& RS : Slot.RandomStats)
		{
			switch (RS.RandomStatType)
			{
			case RANDOM_STAT_TYPE::ATK:      
				Total.ATK += (int16)RS.RandomStatValue; 
				break;
			case RANDOM_STAT_TYPE::DEF:      
				Total.DEF += (int16)RS.RandomStatValue; 
				break;
			case RANDOM_STAT_TYPE::MAX_HP:   
				Total.MaxHP += (int16)RS.RandomStatValue; 
				break;
			case RANDOM_STAT_TYPE::MAX_MP:   
				Total.MaxMP += (int16)RS.RandomStatValue; 
				break;
			case RANDOM_STAT_TYPE::HP_REGEN: 
				Total.HPRegenPerSec += RS.RandomStatValue;        
				break;
			case RANDOM_STAT_TYPE::MP_REGEN: 
				Total.MPRegenPerSec += RS.RandomStatValue;        
				break;
			default: break;
			}
		}
	}

	return Total;
}

bool UM1ItemManager::IsSlotOnCoolTime(uint8 SlotType, int32 SlotIndex) const
{
	const int32 Key = MakeSlotCoolTimeKey(static_cast<SLOT_TYPE>(SlotType), static_cast<int16>(SlotIndex));
	return SlotCoolTimeMap.Contains(Key);
}

float UM1ItemManager::GetSlotCoolTimeRatio(uint8 SlotType, int32 SlotIndex) const
{
	const int32 Key = MakeSlotCoolTimeKey(static_cast<SLOT_TYPE>(SlotType), static_cast<int16>(SlotIndex));

	const FSlotCoolTimeInfo* Info = SlotCoolTimeMap.Find(Key);
	if (!Info)
		return 0.f;

	if (Info->Duration <= 0.f)
		return 0.f;

	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World)
		return 0.f;

	const float Remaining = FMath::Max(0.f, (float)(Info->EndTime - World->GetTimeSeconds()));
	return Remaining / Info->Duration;
}

int32 UM1ItemManager::MakeSlotCoolTimeKey(SLOT_TYPE SlotType, int16 SlotIndex) const
{
	return ((int32)SlotType << 16) | (uint16)SlotIndex;
}

void UM1ItemManager::StartSlotCoolTime(SLOT_TYPE SlotType, int16 SlotIndex, ITEM_ID UsedItemID)
{
	// 소모품 타입 아니면 리턴
	const ItemData* Data = FM1ItemTable::GetItemData(UsedItemID);
	if (!Data || Data->itemType != ITEM_TYPE::CONSUMABLE)
		return;

	float DurationSec = 0.f;

	// 소모품 타입에 따라 기간 설정
	switch (Data->consumableType)
	{
	case CONSUMABLE_ITEM_TYPE::SMALL_HP_POTION:
		DurationSec = ConsumableConst::HP_POTION_USE_COOLTIME_MS / 1000.f;
		break;

	case CONSUMABLE_ITEM_TYPE::SMALL_MP_POTION:
		DurationSec = ConsumableConst::MP_POTION_USE_COOLTIME_MS / 1000.f;
		break;

	default:
		return;
	}


	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World)
		return;

	// 해당 슬롯 타입 및 index를 통해 key 생성
	const int32 Key = MakeSlotCoolTimeKey(SlotType, SlotIndex);

	// 이미 해당 슬롯에 쿨타임 정보가 있으면 기존 타이머 제거
	if (FSlotCoolTimeInfo* OldInfo = SlotCoolTimeMap.Find(Key))
	{
		World->GetTimerManager().ClearTimer(OldInfo->TimerHandle);
	}

	// 쿨타임 정보 Map에 저장. FindOrAdd를 통해 없으면 정보 생성함.
	FSlotCoolTimeInfo& Info = SlotCoolTimeMap.FindOrAdd(Key);
	Info.Duration = DurationSec;
	Info.EndTime = World->GetTimeSeconds() + DurationSec;

	// 델리게이트 만들어서 람다 등록. 해당 람다가 타이머 지나서 호출되면 쿨타임 Map에서 제거됨.
	FTimerDelegate Delegate;
	Delegate.BindLambda([this, Key]()
		{
			SlotCoolTimeMap.Remove(Key);
		});

	// 타이머 등록해서 DurationSec 후에 델리게이트에 등록한 람다 호출되면서 Map에서 Key에 대한 쿨타임 정보 제거
	World->GetTimerManager().SetTimer(
		Info.TimerHandle,
		Delegate,
		DurationSec,
		false
	);

	OnSlotCoolTimeStart.Broadcast((uint8)SlotType, SlotIndex, DurationSec);
}
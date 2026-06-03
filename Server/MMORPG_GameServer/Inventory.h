#pragma once
#include "ContentsType.h"
#include "ContentsDefine.h"
#include "ContentsStruct.h"

class CUserItemStorage;

class Inventory
{
public:
	Inventory() = default;
	~Inventory() = default;

	void Init(CUserItemStorage* pStorage);
	void Destroy();

	bool ItemSlotChange(uint16 fromIndex , uint16 toIndex);
	bool InsertItemToSlot(ITEM_UID Item, uint16 slotIndex);
	bool DeleteInventorySlot(uint16 slotIndex);

	bool   SlotIndexRangeCheck(uint16 slotIndex);
	uint16 GetEmptySlotIndex();

private:
	ITEM_UID            m_inventory[UserInventory::INVENTORY_SLOT_MAX] = {ItemUID::ITEM_UID_INVALID_ID};
	CUserItemStorage*   m_pStorage = nullptr;
	std::stack<uint16>  m_slotIndexAllocator;
};


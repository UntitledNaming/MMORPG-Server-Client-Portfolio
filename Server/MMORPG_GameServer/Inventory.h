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

	bool ItemSlotChange(int16 fromIndex , int16 toIndex);
	bool InsertItemToSlot(ITEM_UID Item, int16 slotIndex);
	bool DeleteInventorySlot(int16 slotIndex, ITEM_UID& OutItemUID);
	void EraseEmptyIndex(int16 emptyIndex);
	void     ReturnSlotIndex(int16 slotIndex);
	ITEM_ID  GetItemUID(int16 slotIndex);
	int16    GainEmptySlotIndex();
	uint8    GetUseCount() const { return m_useCount; };

	const std::array<ITEM_UID, UserInventory::INVENTORY_SLOT_MAX>& GetInventoryArray() const { return m_inventory; };

private:
	bool   IndexRangeCheck(int16 slotIndex);

private:
	CUserItemStorage*                                          m_pStorage = nullptr;
	uint8                                                      m_useCount = 0;
	std::array<ITEM_UID, UserInventory::INVENTORY_SLOT_MAX>    m_inventory;
	std::set<int16>                                            m_slotIndexAllocator;
};


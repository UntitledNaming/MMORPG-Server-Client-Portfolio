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
	bool InsertNotFullStackItemUID(ITEM_ID itemID, ITEM_UID InItemUID);
	bool DeleteNotFullStackItemUID(ITEM_ID itemID, ITEM_UID InItemUID);
	void FindNotFullStackItemUID(ITEM_ID itemID, ITEM_UID& outItemUID);

	bool     GetItemUID(int16 slotIndex, ITEM_UID& OutItemUID);
	int16    GetEmptySlotIndex();
	int16    GetUIDToSlotIndex(ITEM_UID uid);

private:
	bool   IndexRangeCheck(int16 slotIndex);

private:
	ITEM_UID                                                   m_inventory[UserInventory::INVENTORY_SLOT_MAX];
	CUserItemStorage*                                          m_pStorage = nullptr;
	std::stack<int16>                                          m_slotIndexAllocator;
	std::unordered_map<ITEM_ID, std::unordered_set<ITEM_UID>>  m_stackableItemUIDs;
	std::unordered_map<ITEM_UID, int16>                        m_uidToSlotIndex;
};


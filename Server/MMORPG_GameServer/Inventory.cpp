#include <stack>
#include <unordered_map>
#include <unordered_set>
#include "MemoryPoolTLS.h"
#include "ItemUIDAllocator.h"
#include "CUserItemStorage.h"
#include "Inventory.h"

void Inventory::Init(CUserItemStorage* pStorage)
{
	for (int i = 0; i < UserInventory::INVENTORY_SLOT_MAX; i++)
	{
		m_inventory[i] = ItemUID::ITEM_UID_INVALID_ID;
	}

	m_pStorage = pStorage;
	m_stackableItemUIDs.clear();
	m_uidToSlotIndex.clear();

	// DB에 저장된 아이템들 인벤토리에 배치

	// slotIndexAllocator 세팅

	// 전체 인벤토리 돌면서 stackableItemUIDs 세팅
}

void Inventory::Destroy()
{
	for (int i = 0; i < UserInventory::INVENTORY_SLOT_MAX; i++)
	{
		m_inventory[i] = ItemUID::ITEM_UID_INVALID_ID;
	}

	while (!m_slotIndexAllocator.empty())
	{
		m_slotIndexAllocator.pop();
	}

	m_pStorage = nullptr;
}

bool Inventory::ItemSlotChange(int16 fromIndex, int16 toIndex)
{
	if (!(IndexRangeCheck(fromIndex) && IndexRangeCheck(toIndex)))
		return false;

	ITEM_UID TempTo = m_inventory[toIndex];
	ITEM_UID TempFrom = m_inventory[fromIndex];

	m_inventory[toIndex] = TempFrom;
	m_inventory[fromIndex] = TempTo;

	// 기존 from, to에 있는 UID들의 index 변경
	std::unordered_map<ITEM_UID, int16>::iterator itTo;

	itTo = m_uidToSlotIndex.find(TempTo);
	if (itTo != m_uidToSlotIndex.end())
	{
		itTo->second = fromIndex;
	}

	std::unordered_map<ITEM_UID, int16>::iterator itFrom;
	itFrom = m_uidToSlotIndex.find(TempFrom);
	if (itFrom != m_uidToSlotIndex.end())
	{
		itFrom->second = toIndex;
	}


	return true;
}

// Init에서 아이템 인벤토리에 배치하거나 새로운 아이템 생성되거나 장착 아이템 빼서 인벤토리에 배치시
bool Inventory::InsertItemToSlot(ITEM_UID Item, int16 slotIndex)
{
	if (!IndexRangeCheck(slotIndex))
		return false;

	m_inventory[slotIndex] = Item;
	m_uidToSlotIndex.insert(std::pair<ITEM_UID, int16>(Item, slotIndex));
	return true;
}

bool Inventory::DeleteInventorySlot(int16 slotIndex, ITEM_UID& OutItemUID)
{
	if (!IndexRangeCheck(slotIndex))
		return false;

	OutItemUID = m_inventory[slotIndex];
	m_inventory[slotIndex] = ItemUID::ITEM_UID_INVALID_ID;

	// 아이템 UID 존재시 index 관리 자료구조에서 제거
	if (OutItemUID != ItemUID::ITEM_UID_INVALID_ID)
	{
		m_uidToSlotIndex.erase(OutItemUID);
	}

	return true;
}

bool Inventory::InsertNotFullStackItemUID(ITEM_ID itemID, ITEM_UID InItemUID)
{
	std::unordered_map<ITEM_ID, std::unordered_set<ITEM_UID>>::iterator it = m_stackableItemUIDs.find(itemID);

	if (it == m_stackableItemUIDs.end())
		return false;

	it->second.insert(InItemUID);
	return true;
}

bool Inventory::DeleteNotFullStackItemUID(ITEM_ID itemID, ITEM_UID InItemUID)
{
	std::unordered_map<ITEM_ID, std::unordered_set<ITEM_UID>>::iterator it = m_stackableItemUIDs.find(itemID);
	if (it == m_stackableItemUIDs.end())
		return false;

	std::unordered_set<ITEM_UID>::iterator it2 = it->second.find(InItemUID);
	if (it2 == it->second.end())
		return false;

	it->second.erase(it2);
	return true;
}

void Inventory::FindNotFullStackItemUID(ITEM_ID itemID, ITEM_UID& outItemUID)
{
	outItemUID = ItemUID::ITEM_UID_INVALID_ID;

	std::unordered_map<ITEM_ID, std::unordered_set<ITEM_UID>>::iterator it = m_stackableItemUIDs.find(itemID);
	if (it == m_stackableItemUIDs.end())
		return;

	if (it->second.empty())
		return;

	outItemUID = *(it->second.begin());
}

bool Inventory::IndexRangeCheck(int16 slotIndex)
{
	if (slotIndex < 0 || slotIndex >= UserInventory::INVENTORY_SLOT_MAX)
		return false;

	return true;
}

bool Inventory::GetItemUID(int16 slotIndex, ITEM_UID& OutItemUID)
{
	OutItemUID = ItemUID::ITEM_UID_INVALID_ID;

	if (!IndexRangeCheck(slotIndex))
		return false;

	if (m_inventory[slotIndex] == ItemUID::ITEM_UID_INVALID_ID)
		return false;

	OutItemUID = m_inventory[slotIndex];
	return true;
}

int16 Inventory::GetEmptySlotIndex()
{
	if (m_slotIndexAllocator.empty())
		return -1;

	int16 ret = m_slotIndexAllocator.top();
	m_slotIndexAllocator.pop();

	return ret;
}

int16 Inventory::GetUIDToSlotIndex(ITEM_UID uid)
{
	std::unordered_map<ITEM_UID, int16>::iterator it = m_uidToSlotIndex.find(uid);
	if (it == m_uidToSlotIndex.end())
		return -1;

	return it->second;
}

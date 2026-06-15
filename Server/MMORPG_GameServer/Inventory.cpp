#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <array>
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
	m_uidToSlotIndex.clear();

	// DB에 저장된 아이템들 인벤토리에 배치

	// slotIndexAllocator 세팅
	for (int i = 0; i < UserInventory::INVENTORY_SLOT_MAX; i++)
	{
		m_slotIndexAllocator.insert(i);
	}
	// 전체 인벤토리 돌면서 stackableItemUIDs 세팅
}

void Inventory::Destroy()
{
	for (int i = 0; i < UserInventory::INVENTORY_SLOT_MAX; i++)
	{
		m_inventory[i] = ItemUID::ITEM_UID_INVALID_ID;
	}

	m_slotIndexAllocator.clear();

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

	if (m_inventory[slotIndex] == ItemUID::ITEM_UID_INVALID_ID)
		__debugbreak();

	std::unordered_set<int16>::iterator it = m_slotIndexAllocator.find(slotIndex);
	if (it != m_slotIndexAllocator.end())
		__debugbreak();

	m_inventory[slotIndex] = Item;
	m_uidToSlotIndex.insert(std::pair<ITEM_UID, int16>(Item, slotIndex));
	m_useCount++;
	return true;
}

bool Inventory::DeleteInventorySlot(int16 slotIndex, ITEM_UID& OutItemUID)
{
	if (!IndexRangeCheck(slotIndex))
		return false;

	if (m_inventory[slotIndex] == ItemUID::ITEM_UID_INVALID_ID)
		return false;

	OutItemUID = m_inventory[slotIndex];
	m_inventory[slotIndex] = ItemUID::ITEM_UID_INVALID_ID;

	m_uidToSlotIndex.erase(OutItemUID);
	m_useCount--;
	m_slotIndexAllocator.insert(slotIndex);
	return true;
}

bool Inventory::IndexRangeCheck(int16 slotIndex)
{
	if (slotIndex < 0 || slotIndex >= UserInventory::INVENTORY_SLOT_MAX)
		return false;

	return true;
}

void Inventory::InsertSlotIndex(int16 slotIndex)
{
	m_slotIndexAllocator.insert(slotIndex);
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

int16 Inventory::GainEmptySlotIndex()
{
	if (m_slotIndexAllocator.empty())
		return -1;

	std::unordered_set<int16>::iterator it = m_slotIndexAllocator.begin();
	int16 ret = *it;
	m_slotIndexAllocator.erase(it);
	return ret;
}

int16 Inventory::GetUIDToSlotIndex(ITEM_UID uid)
{
	std::unordered_map<ITEM_UID, int16>::iterator it = m_uidToSlotIndex.find(uid);
	if (it == m_uidToSlotIndex.end())
		return -1;

	return it->second;
}

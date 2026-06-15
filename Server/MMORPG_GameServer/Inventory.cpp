#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <set>
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

	// to가 InvalidID인 경우 index 반환 및 사용할 index 제거
	// from, to 모두 아이템이 있으면 index 반환 필요x
	if (m_inventory[toIndex] == ItemUID::ITEM_UID_INVALID_ID)
	{
		// to가 Invalid면 fromindex가 empty가 되고 toindex가 할당 자료구조에서 제거되어야 함.
		m_slotIndexAllocator.erase(toIndex);
		m_slotIndexAllocator.insert(fromIndex);
	}

	// UID Swap 작업
	ITEM_UID TempTo = m_inventory[toIndex];

	m_inventory[toIndex] = m_inventory[fromIndex];
	m_inventory[fromIndex] = TempTo;

	return true;
}

// Init에서 아이템 인벤토리에 배치하거나 새로운 아이템 생성되거나 장착 아이템 빼서 인벤토리에 배치시
bool Inventory::InsertItemToSlot(ITEM_UID Item, int16 slotIndex)
{
	if (!IndexRangeCheck(slotIndex))
		return false;

	std::set<int16>::iterator it = m_slotIndexAllocator.find(slotIndex);
	if (it != m_slotIndexAllocator.end())
		__debugbreak();

	if (m_inventory[slotIndex] != ItemUID::ITEM_UID_INVALID_ID)
		__debugbreak();

	m_inventory[slotIndex] = Item;
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

	m_useCount--;
	return true;
}

void Inventory::EraseEmptyIndex(int16 emptyIndex)
{
	m_slotIndexAllocator.erase(emptyIndex);
}

bool Inventory::IndexRangeCheck(int16 slotIndex)
{
	if (slotIndex < 0 || slotIndex >= UserInventory::INVENTORY_SLOT_MAX)
		return false;

	return true;
}

void Inventory::ReturnSlotIndex(int16 slotIndex)
{
	m_slotIndexAllocator.insert(slotIndex);
}

ITEM_ID Inventory::GetItemUID(int16 slotIndex)
{
	if (!IndexRangeCheck(slotIndex))
		return false;

	return m_inventory[slotIndex];
}

int16 Inventory::GainEmptySlotIndex()
{
	if (m_slotIndexAllocator.empty())
		return -1;

	std::set<int16>::iterator it = m_slotIndexAllocator.begin();
	int16 ret = *it;
	m_slotIndexAllocator.erase(it);
	return ret;
}


#include <stack>
#include "MemoryPoolTLS.h"
#include "ItemUIDAllocator.h"
#include "CUserItemStorage.h"
#include "Inventory.h"

void Inventory::Init(CUserItemStorage* pStorage)
{
	m_pStorage = pStorage;

	// DB에 저장된 아이템들 인벤토리에 배치

	// slotIndexAllocator 세팅
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

bool Inventory::ItemSlotChange(uint16 fromIndex, uint16 toIndex)
{
	if (!(SlotIndexRangeCheck(fromIndex) && SlotIndexRangeCheck(toIndex)))
		return false;

	ITEM_UID Temp = m_inventory[toIndex];
	m_inventory[toIndex] = m_inventory[fromIndex];
	m_inventory[fromIndex] = Temp;

	return true;
}

// Init에서 아이템 인벤토리에 배치하거나 새로운 아이템 생성되거나 장착 아이템 빼서 인벤토리에 배치시
bool Inventory::InsertItemToSlot(ITEM_UID Item, uint16 slotIndex)
{
	if (!SlotIndexRangeCheck(slotIndex))
		return false;

	m_inventory[slotIndex] = Item;

	return true;
}

bool Inventory::DeleteInventorySlot(uint16 slotIndex)
{
	if (!SlotIndexRangeCheck(slotIndex))
		return false;

	m_inventory[slotIndex] = ItemUID::ITEM_UID_INVALID_ID;
	return true;
}

bool Inventory::SlotIndexRangeCheck(uint16 slotIndex)
{
	if (slotIndex < 0 || slotIndex >= UserInventory::INVENTORY_SLOT_MAX)
		return false;

	return true;
}

uint16 Inventory::GetEmptySlotIndex()
{
	if (m_slotIndexAllocator.empty())
		return -1;

	uint16 ret = m_slotIndexAllocator.top();
	m_slotIndexAllocator.pop();

	return ret;
}

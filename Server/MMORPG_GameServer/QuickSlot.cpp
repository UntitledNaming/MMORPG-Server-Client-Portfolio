#include "QuickSlot.h"

void QuickSlot::Init()
{
	for (int i = 0; i < UserQuickSlot::QUICK_SLOT_MAX; i++)
	{
		m_quickSlot[i] = ItemUID::ITEM_UID_INVALID_ID;
	}
}

void QuickSlot::Destroy()
{
	for (int i = 0; i < UserQuickSlot::QUICK_SLOT_MAX; i++)
	{
		m_quickSlot[i] = ItemUID::ITEM_UID_INVALID_ID;
	}
}

bool QuickSlot::SetConsumable(int16 slotIndex, ITEM_UID InItemUID, ITEM_UID& OutItemUID)
{
	if (!IndexRangeCheck(slotIndex))
		return false;

	OutItemUID = m_quickSlot[slotIndex];
	m_quickSlot[slotIndex] = InItemUID;

	return true;
}

bool QuickSlot::ClearConsumable(int16 slotIndex, ITEM_UID& OutItemUID)
{
	if (!IndexRangeCheck(slotIndex))
		return false;

	OutItemUID = m_quickSlot[slotIndex];
	m_quickSlot[slotIndex] = ItemUID::ITEM_UID_INVALID_ID;

	return true;
}

bool QuickSlot::SwapSlot(int16 fromIndex, int16 toIndex)
{
	if (!(IndexRangeCheck(fromIndex) && IndexRangeCheck(toIndex)))
		return false;

	ITEM_UID Temp = m_quickSlot[toIndex];
	m_quickSlot[toIndex] = m_quickSlot[fromIndex];
	m_quickSlot[fromIndex] = Temp;

	return true;
}

ITEM_UID QuickSlot::GetQuickSlotItem(int16 index)
{
	if (!IndexRangeCheck(index))
		return ItemUID::ITEM_UID_INVALID_ID;

	return m_quickSlot[index];
}

bool QuickSlot::IndexRangeCheck(int16 slotIndex)
{
	if (slotIndex < 0 || slotIndex >= UserQuickSlot::QUICK_SLOT_MAX)
		return false;

	return true;
}

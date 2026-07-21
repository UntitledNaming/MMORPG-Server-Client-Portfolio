#include <windows.h>
#include "ItemUIDAllocator.h"

UIDRange ItemUIDAllocator::m_curRange{};
BOOL ItemUIDAllocator::m_itemUIDAlloc = false;

void   ItemUIDAllocator::Init(ITEM_UID startUID, ITEM_UID nextStartUID)
{
	m_curRange.allocID = startUID;
	m_curRange.lastID = nextStartUID - 1;
	m_itemUIDAlloc = true;
}

ITEM_UID   ItemUIDAllocator::Alloc()
{
	if (m_curRange.allocID > m_curRange.lastID)
		return ItemUID::ITEM_UID_INVALID_ID;

	return InterlockedIncrement64((long long*) & m_curRange.allocID);
}
#include <windows.h>
#include "ItemUIDAllocator.h"

UIDRange ItemUIDAllocator::m_curRange{};

void   ItemUIDAllocator::Init(ITEM_UID startUID, ITEM_UID nextStartUID)
{
	m_curRange.allocID = startUID;
	m_curRange.lastID = nextStartUID - 1;
}

ITEM_UID   ItemUIDAllocator::Alloc()
{
	if (m_curRange.allocID > m_curRange.lastID)
		return ItemUID::ITEM_UID_INVALID_ID;

	// todo : 추후 여러 그룹에서 사용시 Interlock 필요
	return m_curRange.allocID++;
}
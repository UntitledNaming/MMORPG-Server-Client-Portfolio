#include <windows.h>
#include "ItemUIDAllocator.h"

UIDRange ItemUIDAllocator::m_curRange{};

void   ItemUIDAllocator::Init()
{
	// todo : DB 접근해서 UID 가져와서 세팅하기
	m_curRange.allocID = 1;
	m_curRange.lastID = ItemUID::ITEM_UID_RESERVE_COUNT;
}

ITEM_UID   ItemUIDAllocator::Alloc()
{
	if (m_curRange.allocID > m_curRange.lastID)
		return ItemUID::ITEM_UID_INVALID_ID;

	return m_curRange.allocID++;
}
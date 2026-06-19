#pragma once
#include "ContentsType.h"
#include "ContentsDefine.h"
#include "ContentsStruct.h"


class ItemUIDAllocator
{
public:
	ItemUIDAllocator() = default;
	~ItemUIDAllocator() = default;

    static void     Init(ITEM_UID startUID, ITEM_UID nextStartUID);

	static ITEM_UID Alloc();

private:
	static UIDRange m_curRange;
};


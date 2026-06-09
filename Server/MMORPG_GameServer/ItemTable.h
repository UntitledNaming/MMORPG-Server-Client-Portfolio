#pragma once
#include "ContentsType.h"
#include "ContentsStruct.h"

class ItemTable
{
public:
	ItemTable() = default;
	~ItemTable() = default;

	static void  Init();
	static void  Destroy();
	static const ItemData* GetItemData(ITEM_ID ItemID);

private:
	static std::unordered_map<ITEM_ID, ItemData*> m_itemTable;
	static CMPoolTLS<ItemData>*                   m_pItemDataPool;
};


#pragma once

#include "CoreMinimal.h"
#include <unordered_map>
#include "ContentsType.h"
#include "ContentsStruct.h"

template <typename T>
class CMPoolTLS;

class M1_API FM1ItemTable 
{
public:

	FM1ItemTable() = default;
	~FM1ItemTable() = default;

	static void         Init();
	static void         Destroy();
	static const ItemData* GetItemData(ITEM_ID ItemID);
	static FString      GetItemName(ITEM_ID ItemID);

private:
	static std::unordered_map<ITEM_ID, ItemData*> m_ItemTable;
	static CMPoolTLS<ItemData>*                   m_pItemDataPool;
};

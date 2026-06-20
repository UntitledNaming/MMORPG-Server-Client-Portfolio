#pragma once
#include "ContentsStruct.h"

struct UserItem;

template <typename T>
class CMPoolTLS;

class CDBManager;

class CUserItemStorage
{
public:
	CUserItemStorage() = default;
	~CUserItemStorage() = default;

	// 서버 가동시 한번 호출
	static void ItemPoolInit();
	static void ItemPoolDestroy();

	void            Init();
	void            Destroy();
	void            LoadItemFromDB(const ItemLoadData& Info);
	void            ExchangeSlotInfo(ITEM_UID itemUID, SLOT_TYPE slotType, int16 slotindex);
	void            SetItemDirtyFlag(ITEM_UID itemUID, bool flag);
	bool            CreateItem(const BaseItemInfo& Info, ITEM_UID& OutItemUID);
	bool            DeleteItem(ITEM_UID ItemUID);
	bool            ChangeItemCount(ITEM_UID ItemUID, uint16 NewCount);
	bool            CollectDirtyItems(std::vector<ItemSlotUpdateData>& OutItems);
	bool            IsStorageEmpty() { return m_storage.empty(); }
	const UserItem* FindItem(ITEM_UID ItemUID) const;
	uint16          GetItemCount(ITEM_UID InItemUID);

private:
	std::unordered_map<ITEM_UID, UserItem*>   m_storage;
	static CMPoolTLS<UserItem>*               m_itemPool;
};


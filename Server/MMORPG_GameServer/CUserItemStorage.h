#pragma once

struct UserItem;

class CUserItemStorage
{
public:
	CUserItemStorage() = default;
	~CUserItemStorage() = default;


	static void ItemPoolInit();
	static void ItemPoolDestroy();

	bool            CreateItem(const ItemCreateInfo& Info, ITEM_UID& OutItemUID);
	bool            DeleteItem(ITEM_UID ItemUID);
	bool            ChangeItemCount(ITEM_UID ItemUID, uint16 NewCount);
	const UserItem* FindItem(ITEM_UID ItemUID);
	void            LoadItemFromDB(const DBItemInfo& Info);
	void            StorageDestroy();

private:
	std::unordered_map<ITEM_UID, UserItem*>   m_storage;
	static CMPoolTLS<UserItem>*               m_itemPool;
};


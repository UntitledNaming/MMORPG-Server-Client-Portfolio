#pragma once

struct UserItem;

template <typename T>
class CMPoolTLS;

class CUserItemStorage
{
public:
	CUserItemStorage() = default;
	~CUserItemStorage() = default;


	static void ItemPoolInit();
	static void ItemPoolDestroy();

	void            Init();
	void            Destroy();
	void            LoadItemFromDB(const ItemLoadData& Info);
	bool            CreateItem(const BaseItemInfo& Info, ITEM_UID& OutItemUID);
	bool            DeleteItem(ITEM_UID ItemUID);
	bool            ChangeItemCount(ITEM_UID ItemUID, uint16 NewCount);
	const UserItem* FindItem(ITEM_UID ItemUID) const;
	uint16          GetItemCount(ITEM_UID InItemUID);

private:
	std::unordered_map<ITEM_UID, UserItem*>   m_storage;
	static CMPoolTLS<UserItem>*               m_itemPool;
};


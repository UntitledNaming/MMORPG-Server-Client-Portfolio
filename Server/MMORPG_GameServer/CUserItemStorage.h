#pragma once

struct UserItem;

class CUserItemStorage
{
public:
	CUserItemStorage() = default;
	~CUserItemStorage() = default;

	static void ItemPoolInit();
	static void ItemPoolDestory();

	void CreateItem(const ItemCreateInfo& Info);
	bool DeleteItem(uint64 ItemUID);
	void LoadItemFromDB(const DBItemInfo& Info);

private:
	std::unordered_map<uint64, UserItem*>   m_storage;
public:
	static CMPoolTLS<UserItem>*             m_itemPool;
};


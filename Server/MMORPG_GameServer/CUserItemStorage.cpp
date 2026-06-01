#include <unordered_map>
#include "ContentsType.h"
#include "ContentsStruct.h"
#include "ContentsDefine.h"
#include "MemoryPoolTLS.h"
#include "CUserItemStorage.h"

CMPoolTLS<UserItem>* CUserItemStorage::m_itemPool = nullptr;

void CUserItemStorage::ItemPoolInit()
{
	if (m_itemPool)
		return;

	m_itemPool = new CMPoolTLS<UserItem>;
}

void CUserItemStorage::ItemPoolDestory()
{
	delete m_itemPool;
}

void CUserItemStorage::CreateItem(const ItemCreateInfo& Info)
{
	UserItem* pItem = m_itemPool->Alloc();

	pItem->m_count = Info.count;
}

bool CUserItemStorage::DeleteItem(uint64 ItemUID)
{
	std::unordered_map<uint64, UserItem*>::iterator it = m_storage.find(ItemUID);

}

void CUserItemStorage::LoadItemFromDB(const DBItemInfo& Info)
{
	UserItem* pItem = m_itemPool->Alloc();



}

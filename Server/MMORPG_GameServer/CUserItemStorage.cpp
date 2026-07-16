#include <unordered_map>
#include <thread>
#include "ContentsType.h"
#include "ContentsStruct.h"
#include "ContentsDefine.h"
#include "MemoryPoolTLS.h"
#include "ItemUIDAllocator.h"
#include "SectorPos.h"
#include "FieldDropItemPool.h"
#include "ItemTable.h"
#include "CUserItemStorage.h"

CMPoolTLS<UserItem>* CUserItemStorage::m_itemPool = nullptr;

void CUserItemStorage::ItemPoolInit()
{
	if (m_itemPool)
		return;

	m_itemPool = new CMPoolTLS<UserItem>;
}

void CUserItemStorage::ItemPoolDestroy()
{
	delete m_itemPool;
	m_itemPool = nullptr;
}

void CUserItemStorage::Init()
{
	m_storage.clear();
}

void CUserItemStorage::Destroy()
{
	std::unordered_map<ITEM_UID, UserItem*>::iterator it = m_storage.begin();
	for (; it != m_storage.end(); ++it)
	{
		m_itemPool->Free(it->second);
	}

	m_storage.clear();
}

bool CUserItemStorage::CreateItem(const BaseItemInfo& Info, ITEM_UID& OutItemUID)
{
	if (m_storage.size() >= UserItemStorage::MAX_ITEM_STORAGE_COUNT)
		return false;

	UserItem* pItem = m_itemPool->Alloc();

	ITEM_UID uid = ItemUIDAllocator::Alloc();
	if (uid == ItemUID::ITEM_UID_INVALID_ID)
	{
		m_itemPool->Free(pItem);
		OutItemUID = ItemUID::ITEM_UID_INVALID_ID;
		return false;
	}

	pItem->itemUID = uid;
	pItem->count = Info.count;
	pItem->itemID = Info.itemID;
	pItem->randomStatCount = Info.randomStatCount;
	pItem->dirtyFlag = false;
	pItem->itemUID = uid;
	for (int i = 0; i < Info.randomStatCount; i++)
	{
		pItem->randomStat[i].randomStatType = Info.randomStat[i].randomStatType;
		pItem->randomStat[i].randomStatValue = Info.randomStat[i].randomStatValue;
	}

	m_storage.insert(std::pair<ITEM_UID, UserItem*>(uid, pItem));

	OutItemUID = uid;
	return true;
}

bool CUserItemStorage::DeleteItem(ITEM_UID ItemUID)
{
	std::unordered_map<ITEM_UID, UserItem*>::iterator it = m_storage.find(ItemUID);

	// 없는 아이템 삭제하려함.
	if (it == m_storage.end())
		return false;

	UserItem* pItem = it->second;
	m_storage.erase(it);

	m_itemPool->Free(pItem);
	return true;
}

bool CUserItemStorage::ChangeItemCount(ITEM_UID ItemUID, uint16 NewCount)
{
	std::unordered_map<ITEM_UID, UserItem*>::iterator it = m_storage.find(ItemUID);
	if (it == m_storage.end())
		return false;

	it->second->count = NewCount;
	return true;
}

bool CUserItemStorage::CollectDirtyItems(std::vector<ItemSlotUpdateData>& OutStackItems, std::vector<ItemSlotUpdateData>& OutInstanceItems)
{
	if (m_storage.empty())
		return false;

	std::unordered_map<ITEM_UID, UserItem*>::iterator it = m_storage.begin();
	for (; it != m_storage.end(); ++it)
	{
		UserItem* pItem = it->second;
		if (!pItem->dirtyFlag)
			continue;

		const ItemData* pItemData = ItemTable::GetItemData(pItem->itemID);
		if (pItemData == nullptr)
			__debugbreak();


		// 플래그 켜져 있으면  Job에 넣기
		ItemSlotUpdateData item = {};
		item.itemUID = pItem->itemUID;
		item.slotType = pItem->slotType;
		item.slotIndex = pItem->slotIndex;
		
		if (pItemData->itemType == ITEM_TYPE::CONSUMABLE)
		{
			OutStackItems.push_back(item);
		}
		else if (pItemData->itemType == ITEM_TYPE::EQUIPMENT)
		{
			OutInstanceItems.push_back(item);
		}

		pItem->dirtyFlag = false;
	}

	return true;
}

const UserItem* CUserItemStorage::FindItem(ITEM_UID ItemUID) const
{
	std::unordered_map<ITEM_UID, UserItem*>::const_iterator it = m_storage.find(ItemUID);
	if (it == m_storage.end())
		return nullptr;

	return it->second;
}

void CUserItemStorage::LoadItemFromDB(const ItemLoadData& Info)
{
	UserItem* pItem = m_itemPool->Alloc();

	pItem->itemUID = Info.itemUID;
	pItem->count = Info.count;
	pItem->itemID = Info.itemID;
	pItem->randomStatCount = Info.randomStatCount;
	pItem->dirtyFlag = false;
	pItem->slotType = Info.slotType;
	pItem->slotIndex = Info.slotIndex;

	for (int i = 0; i < Info.randomStatCount; i++)
	{
		pItem->randomStat[i].randomStatType = Info.randomStat[i].randomStatType;
		pItem->randomStat[i].randomStatValue = Info.randomStat[i].randomStatValue;
	}
	m_storage.insert(std::pair<ITEM_UID, UserItem*>(pItem->itemUID, pItem));
}

void CUserItemStorage::ExchangeSlotInfo(ITEM_UID itemUID, SLOT_TYPE slotType, int16 slotindex)
{
	std::unordered_map<ITEM_UID, UserItem*>::iterator it = m_storage.find(itemUID);
	if (it == m_storage.end())
		return;

	UserItem* pItem = it->second;
	pItem->slotType = slotType;
	pItem->slotIndex = slotindex;
}

void CUserItemStorage::SetItemDirtyFlag(ITEM_UID itemUID, bool flag)
{
	std::unordered_map<ITEM_UID, UserItem*>::iterator it = m_storage.find(itemUID);
	if (it == m_storage.end())
		return;

	it->second->dirtyFlag = flag;
}

uint16 CUserItemStorage::GetItemCount(ITEM_UID InItemUID)
{
	std::unordered_map<ITEM_UID, UserItem*>::iterator it = m_storage.find(InItemUID);
	if (it == m_storage.end())
		return 0;

	return it->second->count;
}

#include <vector>
#include <array>
#include <unordered_map>
#include <set>
#include "ContentsStruct.h"
#include "ContentsDefine.h"
#include "MemoryPoolTLS.h"
#include "SectorPos.h"
#include "Inventory.h"
#include "Equipment.h"
#include "QuickSlot.h"
#include "CUserItemStorage.h"
#include "IUser.h"
#include "CUser.h"
#include "CMonster.h"
#include "FieldDropItemPool.h"
#include "FieldSector.h"

void FieldSector::Init()
{
	m_usersCount = 0;
	m_monsterCount = 0;
	m_itemCount = 0;
	m_users.resize(FieldConst::MAX_SECTOR_USER_COUNT);
	m_monsters.resize(FieldConst::MAX_SECTOR_MONSTER_COUNT);
	m_items.resize(FieldConst::MAX_SECTOR_ITEM_COUNT);
}

bool FieldSector::AddUser(CUser* user)
{
	if (m_usersCount >= FieldConst::MAX_SECTOR_USER_COUNT)
		return false;

	m_users[m_usersCount] = user;
	user->SetSectorArrayIdx(m_usersCount);
	m_usersCount++;

	return true;
}

void FieldSector::RemoveUser(CUser* user)
{
	CUser* pOther = m_users[m_usersCount - 1];
	m_users[user->GetSectorArrayIdx()] = pOther;
	pOther->SetSectorArrayIdx(user->GetSectorArrayIdx());
	m_usersCount--;
}

bool FieldSector::AddMonster(CMonster* monster)
{
	if (m_monsterCount >= FieldConst::MAX_SECTOR_MONSTER_COUNT)
		return false;

	m_monsters[m_monsterCount] = monster;
	monster->SetSectorIdx(m_monsterCount);
	m_monsterCount++;

	monster->addcount++;

	return true;
}

void FieldSector::RemoveMonster(CMonster* monster)
{
	CMonster* pOther = m_monsters[m_monsterCount - 1];
	m_monsters[monster->GetSectorIdx()] = pOther;
	pOther->SetSectorIdx(monster->GetSectorIdx());
	m_monsterCount--;

	pOther->removecount++;
}

bool FieldSector::AddItem(FieldDropItem* item)
{
	if (m_itemCount >= FieldConst::MAX_SECTOR_ITEM_COUNT)
		return false;

	m_items[m_itemCount] = item;
	item->sectorIdx = m_itemCount;
	m_itemCount++;

	return true;
}

void FieldSector::RemoveItem(FieldDropItem* item)
{
	FieldDropItem* pOtherItem = m_items[m_itemCount - 1];
	m_items[item->sectorIdx] = pOtherItem;
	pOtherItem->sectorIdx = item->sectorIdx;
	m_itemCount--;
}

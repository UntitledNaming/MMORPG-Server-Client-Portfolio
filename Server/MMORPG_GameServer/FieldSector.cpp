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
	m_users.resize(FieldConst::DEFAULT_SECTOR_USER_COUNT);
	m_monsters.resize(FieldConst::DEFAULT_SECTOR_MONSTER_COUNT);
	m_items.resize(FieldConst::DEFAULT_SECTOR_ITEM_COUNT);
}

void FieldSector::AddUser(CUser* user)
{
	if (m_usersCount >= m_users.size())
		m_users.resize(m_usersCount * 2);

	m_users[m_usersCount] = user;
	user->SetSectorArrayIdx(m_usersCount);
	m_usersCount++;

}

void FieldSector::RemoveUser(CUser* user)
{
	if (m_usersCount == 0)
		return;

	CUser* pOther = m_users[m_usersCount - 1];
	m_users[user->GetSectorArrayIdx()] = pOther;
	pOther->SetSectorArrayIdx(user->GetSectorArrayIdx());
	m_usersCount--;
}

void FieldSector::AddMonster(CMonster* monster)
{
	if (m_monsterCount >= m_monsters.size())
		m_monsters.resize(m_monsterCount * 2);

	m_monsters[m_monsterCount] = monster;
	monster->SetSectorIdx(m_monsterCount);
	m_monsterCount++;

	monster->addcount++;

}

void FieldSector::RemoveMonster(CMonster* monster)
{
	if (m_monsterCount == 0)
		return;

	CMonster* pOther = m_monsters[m_monsterCount - 1];
	m_monsters[monster->GetSectorIdx()] = pOther;
	pOther->SetSectorIdx(monster->GetSectorIdx());
	m_monsterCount--;

	pOther->removecount++;
}

void FieldSector::AddItem(FieldDropItem* item)
{
	if (m_itemCount >= m_items.size())
		m_items.resize(m_itemCount * 2);

	m_items[m_itemCount] = item;
	item->sectorIdx = m_itemCount;
	m_itemCount++;

}

void FieldSector::RemoveItem(FieldDropItem* item)
{
	if (m_itemCount == 0)
		return;

	FieldDropItem* pOtherItem = m_items[m_itemCount - 1];
	m_items[item->sectorIdx] = pOtherItem;
	pOtherItem->sectorIdx = item->sectorIdx;
	m_itemCount--;
}

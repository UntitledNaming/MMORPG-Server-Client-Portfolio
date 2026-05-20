#include <vector>
#include "MemoryPoolTLS.h"
#include "SectorPos.h"
#include "ContentsStruct.h"
#include "ContentsDefine.h"
#include "IUser.h"
#include "CUser.h"
#include "CMonster.h"
#include "FieldSector.h"

void FieldSector::Init()
{
	m_usersCount = 0;
	m_monsterCount = 0;
	m_users.resize(FieldConst::MAX_SECTOR_USER_COUNT);
	m_monsters.resize(FieldConst::MAX_SECTOR_MONSTER_COUNT);
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

	return true;
}

void FieldSector::RemoveMonster(CMonster* monster)
{
	CMonster* pOther = m_monsters[m_monsterCount - 1];
	m_monsters[monster->GetSectorIdx()] = pOther;
	pOther->SetSectorIdx(monster->GetSectorIdx());
	m_monsterCount--;
}
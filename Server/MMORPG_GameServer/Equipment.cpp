#include <unordered_map>
#include "MemoryPoolTLS.h"
#include "ItemUIDAllocator.h"
#include "CUserItemStorage.h"
#include "Equipment.h"

void Equipment::Init(CUserItemStorage* pStorage)
{
	m_pStorage = pStorage;

	// DB에서 장착 장비 긁어와 세팅 및 캐시 데이터 세팅

}
void Equipment::Destroy()
{
	for (int i = 0; i < (int)(EQUIP_SLOT::MAX)-1; i++)
	{
		m_equipment[i] = ItemUID::ITEM_UID_INVALID_ID;
	}
	m_currentATK = 0;
	m_currentDEF = 0;
	m_currentMaxHP = 0;
	m_currentMaxMP = 0;
	m_currentHPRegenPerSec = 0;
	m_currentMPRegenPerSec = 0;
	m_pStorage = nullptr;
}

void Equipment::EquippedItem(EQUIP_SLOT slotNum, ITEM_UID InItemUID, ITEM_UID& OutItemUID)
{
	OutItemUID = m_equipment[(int)slotNum];

	const UserItem* outitem = m_pStorage->FindItem(OutItemUID);
	if (outitem)
	{
		// todo : ItemTable 찾아서 기본 스탯 찾아서 빼기

		// 랜덤 스탯 빼기
		m_currentATK -= outitem->m_randomStat.atk;
		m_currentDEF -= outitem->m_randomStat.def;
		m_currentMaxHP -= outitem->m_randomStat.maxHP;
		m_currentMaxMP -= outitem->m_randomStat.maxMP;
		m_currentHPRegenPerSec -= outitem->m_randomStat.hpRegenPerSec;
		m_currentMPRegenPerSec -= outitem->m_randomStat.mpRegenPerSec;
	}

	// 새로운 아이템 장착 후 스탯 갱신하기
	m_equipment[(int)slotNum] = InItemUID;
	
	const UserItem* initem = m_pStorage->FindItem(InItemUID);
	if (!initem)
		return;

	// todo : ItemTable에서 기본 스탯 찾아서 더하기
	m_currentATK += initem->m_randomStat.atk;
	m_currentDEF += initem->m_randomStat.def;
	m_currentMaxHP += initem->m_randomStat.maxHP;
	m_currentMaxMP += initem->m_randomStat.maxMP;
	m_currentHPRegenPerSec += initem->m_randomStat.hpRegenPerSec;
	m_currentMPRegenPerSec += initem->m_randomStat.mpRegenPerSec;
}

void Equipment::UnEquippedItem(EQUIP_SLOT slotNum, ITEM_UID& OutItemUID)
{
	OutItemUID = m_equipment[(int)slotNum];

	// 캐시 스탯 갱신
	const UserItem* outitem = m_pStorage->FindItem(OutItemUID);
	if (outitem)
	{
		// todo : ItemTable 찾아서 기본 스탯 찾아서 빼기

		// 랜덤 스탯 빼기
		m_currentATK -= outitem->m_randomStat.atk;
		m_currentDEF -= outitem->m_randomStat.def;
		m_currentMaxHP -= outitem->m_randomStat.maxHP;
		m_currentMaxMP -= outitem->m_randomStat.maxMP;
		m_currentHPRegenPerSec -= outitem->m_randomStat.hpRegenPerSec;
		m_currentMPRegenPerSec -= outitem->m_randomStat.mpRegenPerSec;
	}

	m_equipment[(int)slotNum] = ItemUID::ITEM_UID_INVALID_ID;
}

bool Equipment::EquipmentSlotRangeCheck(EQUIP_SLOT slot)
{
	if ((int)slot < (int)EQUIP_SLOT::NONE || (int)slot >= (int)EQUIP_SLOT::MAX)
		return false;

	return true;
}


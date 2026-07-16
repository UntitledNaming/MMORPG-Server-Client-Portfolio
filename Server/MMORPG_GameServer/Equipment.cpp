#include <unordered_map>
#include <array>
#include "MemoryPoolTLS.h"
#include "ItemUIDAllocator.h"
#include "CUserItemStorage.h"
#include "ItemTable.h"
#include "Equipment.h"

void Equipment::Init(CUserItemStorage* pStorage)
{
	for (int i = (int)EQUIP_SLOT::HELMET; i < (int)(EQUIP_SLOT::MAX); i++)
	{
		m_equipment[i] = ItemUID::ITEM_UID_INVALID_ID;
	}

	m_pStorage = pStorage;
	m_useCount = 0;

	m_currentATK = 0;
	m_currentDEF = 0;
	m_currentMaxHP = 0;
	m_currentMaxMP = 0;
	m_currentHPRegenPerSec = 0;
	m_currentMPRegenPerSec = 0;

}
void Equipment::Destroy()
{
	for (int i = (int)EQUIP_SLOT::HELMET; i < (int)(EQUIP_SLOT::MAX); i++)
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

bool Equipment::EquippedItem(EQUIP_SLOT slotNum, ITEM_UID InItemUID, ITEM_UID& OutItemUID)
{
	if (!IndexRangeCheck(slotNum))
		return false;

	OutItemUID = m_equipment[(int)slotNum];

	const UserItem* outitem = m_pStorage->FindItem(OutItemUID);
	if (outitem)
	{
		// ItemTable 찾아서 기본 스탯 찾아서 빼기
		const ItemData* pData = ItemTable::GetItemData(outitem->itemID);

		if (pData)
		{
			m_currentATK -= pData->baseStat.atk;
			m_currentDEF -= pData->baseStat.def;
			m_currentMaxHP -= pData->baseStat.maxHP;
			m_currentMaxMP -= pData->baseStat.maxMP;
			m_currentHPRegenPerSec -= pData->baseStat.hpRegenPerSec;
			m_currentMPRegenPerSec -= pData->baseStat.mpRegenPerSec;
		}
		else
			__debugbreak();

		m_useCount--;

		// 랜덤 스탯 빼기
		uint8 statCount = outitem->randomStatCount;
		for (int i = 0; i < statCount; i++)
		{
			const RandomStatResult& randStat = outitem->randomStat[i];
			switch (randStat.randomStatType)
			{
			case RANDOM_STAT_TYPE::ATK:
				m_currentATK -= randStat.randomStatValue;
				break;

			case RANDOM_STAT_TYPE::DEF:
				m_currentDEF -= randStat.randomStatValue;
				break;

			case RANDOM_STAT_TYPE::MAX_HP:
				m_currentMaxHP -= randStat.randomStatValue;
				break;

			case RANDOM_STAT_TYPE::MAX_MP:
				m_currentMaxMP -= randStat.randomStatValue;
				break;

			case RANDOM_STAT_TYPE::HP_REGEN:
				m_currentHPRegenPerSec -= randStat.randomStatValue;
				break;

			case RANDOM_STAT_TYPE::MP_REGEN:
				m_currentMPRegenPerSec -= randStat.randomStatValue;
				break;
			}
		}
	}

	// 새로운 아이템 장착 후 스탯 갱신하기
	m_equipment[(int)slotNum] = InItemUID;
	
	const UserItem* initem = m_pStorage->FindItem(InItemUID);
	if (!initem)
		__debugbreak();

	// ItemTable에서 기본 스탯 찾아서 더하기
	const ItemData* pInItemData = ItemTable::GetItemData(initem->itemID);
	if (pInItemData)
	{
		m_currentATK += pInItemData->baseStat.atk;
		m_currentDEF += pInItemData->baseStat.def;
		m_currentMaxHP += pInItemData->baseStat.maxHP;
		m_currentMaxMP += pInItemData->baseStat.maxMP;
		m_currentHPRegenPerSec += pInItemData->baseStat.hpRegenPerSec;
		m_currentMPRegenPerSec += pInItemData->baseStat.mpRegenPerSec;
	}
	else
		__debugbreak();

	// 랜덤 스탯 더하기
	uint8 statCount = initem->randomStatCount;
	for (int i = 0; i < statCount; i++)
	{
		const RandomStatResult& randStat = initem->randomStat[i];
		switch (randStat.randomStatType)
		{
		case RANDOM_STAT_TYPE::ATK:
			m_currentATK += randStat.randomStatValue;
			break;

		case RANDOM_STAT_TYPE::DEF:
			m_currentDEF += randStat.randomStatValue;
			break;

		case RANDOM_STAT_TYPE::MAX_HP:
			m_currentMaxHP += randStat.randomStatValue;
			break;

		case RANDOM_STAT_TYPE::MAX_MP:
			m_currentMaxMP += randStat.randomStatValue;
			break;

		case RANDOM_STAT_TYPE::HP_REGEN:
			m_currentHPRegenPerSec += randStat.randomStatValue;
			break;

		case RANDOM_STAT_TYPE::MP_REGEN:
			m_currentMPRegenPerSec += randStat.randomStatValue;
			break;
		}
	}

	m_useCount++;
	return true;
}

bool Equipment::UnEquippedItem(EQUIP_SLOT slotNum, ITEM_UID& OutItemUID)
{
	if (!IndexRangeCheck(slotNum))
		return false;

	OutItemUID = m_equipment[(int)slotNum];
	if (OutItemUID == ItemUID::ITEM_UID_INVALID_ID)
		return false;

	// 캐시 스탯 갱신
	const UserItem* outitem = m_pStorage->FindItem(OutItemUID);
	if (outitem == nullptr)
		return false;

	// ItemTable 찾아서 기본 스탯 찾아서 빼기
	const ItemData* pData = ItemTable::GetItemData(outitem->itemID);
	if (pData)
	{
		m_currentATK -= pData->baseStat.atk;
		m_currentDEF -= pData->baseStat.def;
		m_currentMaxHP -= pData->baseStat.maxHP;
		m_currentMaxMP -= pData->baseStat.maxMP;
		m_currentHPRegenPerSec -= pData->baseStat.hpRegenPerSec;
		m_currentMPRegenPerSec -= pData->baseStat.mpRegenPerSec;
	}
	else
		__debugbreak();

	// 랜덤 스탯 빼기
	uint8 statCount = outitem->randomStatCount;
	for (int i = 0; i < statCount; i++)
	{
		const RandomStatResult& randStat = outitem->randomStat[i];
		switch (randStat.randomStatType)
		{
		case RANDOM_STAT_TYPE::ATK:
			m_currentATK -= randStat.randomStatValue;
			break;

		case RANDOM_STAT_TYPE::DEF:
			m_currentDEF -= randStat.randomStatValue;
			break;

		case RANDOM_STAT_TYPE::MAX_HP:
			m_currentMaxHP -= randStat.randomStatValue;
			break;

		case RANDOM_STAT_TYPE::MAX_MP:
			m_currentMaxMP -= randStat.randomStatValue;
			break;

		case RANDOM_STAT_TYPE::HP_REGEN:
			m_currentHPRegenPerSec -= randStat.randomStatValue;
			break;

		case RANDOM_STAT_TYPE::MP_REGEN:
			m_currentMPRegenPerSec -= randStat.randomStatValue;
			break;
		}
	}


	m_equipment[(int)slotNum] = ItemUID::ITEM_UID_INVALID_ID;
	m_useCount--;
	return true;
}

ITEM_UID Equipment::GetEquippedItem(EQUIP_SLOT slotNum)
{
	if (!IndexRangeCheck(slotNum))
		return ItemUID::ITEM_UID_INVALID_ID;

	return m_equipment[(int)slotNum];
}


bool Equipment::IndexRangeCheck(EQUIP_SLOT slot)
{
	if ((int)slot <= (int)EQUIP_SLOT::NONE || (int)slot >= (int)EQUIP_SLOT::MAX)
		return false;

	return true;
}


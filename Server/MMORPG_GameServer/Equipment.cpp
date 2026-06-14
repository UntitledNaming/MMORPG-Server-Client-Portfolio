#include <unordered_map>
#include <array>
#include "MemoryPoolTLS.h"
#include "ItemUIDAllocator.h"
#include "CUserItemStorage.h"
#include "ItemTable.h"
#include "Equipment.h"

void Equipment::Init(CUserItemStorage* pStorage)
{
	for (int i = 0; i < (int)(EQUIP_SLOT::MAX)-1; i++)
	{
		m_equipment[i] = ItemUID::ITEM_UID_INVALID_ID;
	}

	m_pStorage = pStorage;

	// DB¿¡¼­ ÀåÂø Àåºñ ±Ü¾î¿Í ¼¼ÆÃ ¹× Ä³½Ã µ¥ÀÌÅÍ ¼¼ÆÃ

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

bool Equipment::EquippedItem(EQUIP_SLOT slotNum, ITEM_UID InItemUID, ITEM_UID& OutItemUID)
{
	if (!IndexRangeCheck(slotNum))
		return false;

	OutItemUID = m_equipment[(int)slotNum];

	const UserItem* outitem = m_pStorage->FindItem(OutItemUID);
	if (outitem)
	{
		m_useCount--;

		// ItemTable Ã£¾Æ¼­ ±âº» ½ºÅÈ Ã£¾Æ¼­ »©±â
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


		// ·£´ý ½ºÅÈ »©±â
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

	// »õ·Î¿î ¾ÆÀÌÅÛ ÀåÂø ÈÄ ½ºÅÈ °»½ÅÇÏ±â
	m_equipment[(int)slotNum] = InItemUID;
	
	const UserItem* initem = m_pStorage->FindItem(InItemUID);
	if (!initem)
		return false;

	// ItemTable¿¡¼­ ±âº» ½ºÅÈ Ã£¾Æ¼­ ´õÇÏ±â
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


	// ·£´ý ½ºÅÈ ´õÇÏ±â
	uint8 statCount = outitem->randomStatCount;
	for (int i = 0; i < statCount; i++)
	{
		const RandomStatResult& randStat = outitem->randomStat[i];
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

	// Ä³½Ã ½ºÅÈ °»½Å
	const UserItem* outitem = m_pStorage->FindItem(OutItemUID);
	if (outitem == nullptr)
		return false;

	m_useCount--;

	// ItemTable Ã£¾Æ¼­ ±âº» ½ºÅÈ Ã£¾Æ¼­ »©±â
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


	// ·£´ý ½ºÅÈ »©±â
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
	if ((int)slot < (int)EQUIP_SLOT::NONE || (int)slot >= (int)EQUIP_SLOT::MAX)
		return false;

	return true;
}


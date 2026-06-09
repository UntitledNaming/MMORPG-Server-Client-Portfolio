#pragma once
#include "ContentsType.h"
#include "ContentsDefine.h"
#include "ContentsStruct.h"
#include "ContentsEnum.h"

class CUserItemStorage;

class Equipment
{
public:
	Equipment() = default;
	~Equipment() = default;

	void     Init(CUserItemStorage* pStorage);
	void     Destroy();
			 
	int16    GetATK() const { return m_currentATK; }
	int16    GetDEF() const { return m_currentDEF; }
	int16    GetMaxHP() const { return m_currentMaxHP; }
	int16    GetMaxMP() const { return m_currentMaxMP; }
	uint16   GetHPRegen() const { return m_currentHPRegenPerSec; }
	uint16   GetMPRegen()const { return m_currentMPRegenPerSec; }

	bool     EquippedItem(EQUIP_SLOT slotNum, ITEM_UID InItemUID, ITEM_UID& OutItemUID);
	bool     UnEquippedItem(EQUIP_SLOT slotNum, ITEM_UID& OutItemUID);
	ITEM_UID GetEquippedItem(EQUIP_SLOT slotNum);

private:
	bool     IndexRangeCheck(EQUIP_SLOT slot);

private:
	ITEM_UID          m_equipment[(int)EQUIP_SLOT::MAX];     // Currently Equipped Items;
	CUserItemStorage* m_pStorage = nullptr;

	// Cache Data
	int16     m_currentATK;
	int16     m_currentDEF;
	int16     m_currentMaxHP;
	int16     m_currentMaxMP;
	uint16    m_currentHPRegenPerSec;
	uint16    m_currentMPRegenPerSec;
};


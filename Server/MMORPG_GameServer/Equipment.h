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
	int16    GetHPRegen() const { return m_currentHPRegenPerSec; }
	int16    GetMPRegen()const { return m_currentMPRegenPerSec; }

	bool     EquippedItem(EQUIP_SLOT slotNum, ITEM_UID InItemUID, ITEM_UID& OutItemUID);
	bool     UnEquippedItem(EQUIP_SLOT slotNum, ITEM_UID& OutItemUID);
	ITEM_UID GetEquippedItem(EQUIP_SLOT slotNum);
	uint8    GetUseCount() const { return m_useCount; };
	
	const std::array<ITEM_UID, (int)EQUIP_SLOT::MAX>& GetEquipmentArray() const { return m_equipment; };

private:
	bool     IndexRangeCheck(EQUIP_SLOT slot);

private:
	CUserItemStorage*                          m_pStorage = nullptr;
	uint8                                      m_useCount = 0;
	std::array<ITEM_UID, (int)EQUIP_SLOT::MAX> m_equipment;

	// Cache Data
	int16     m_currentATK;
	int16     m_currentDEF;
	int16     m_currentMaxHP;
	int16     m_currentMaxMP;
	int16     m_currentHPRegenPerSec;
	int16     m_currentMPRegenPerSec;
};


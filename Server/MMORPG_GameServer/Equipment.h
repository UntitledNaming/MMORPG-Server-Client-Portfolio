#pragma once
#include "ContentsDefine.h"
#include "ContentsStruct.h"
#include "ContentsEnum.h"

class Equipment
{
public:
	Equipment() = default;
	~Equipment() = default;

	void Init();
	void Destroy();

	int16   GetATK() const { return m_currentATK; }
	int16   GetDEF() const { return m_currentDEF; }
	int16   GetMaxHP() const { return m_currentMaxHP; }
	int16   GetMaxMP() const { return m_currentMaxMP; }
	uint16  GetHPRegen() const { return m_currentHPRegenPerSec; }
	uint16  GetMPRegen()const { return m_currentMPRegenPerSec; }

private:
	uint64  m_equipment[(int)EQUIP_SLOT::MAX];     // Currently Equi7-87ped Items;

	// Cache Data
	int16   m_currentATK;
	int16   m_currentDEF;
	int16   m_currentMaxHP;
	int16   m_currentMaxMP;
	uint16  m_currentHPRegenPerSec;
	uint16  m_currentMPRegenPerSec;
};


#pragma once

#include "ContentsType.h"
#include "SectorPos.h"
#include "ContentsEnum.h"
#include "ContentsStruct.h"
#include "ContentsDefine.h"


class CMonster
{
public:
	CMonster() = default;
	~CMonster() = default;

	void Init(uint64 monsterID, uint16 monsterType, const Location& spawnLocation);
	void Destroy();
	void Regen();

	const uint64 GetMonsterID() const { return m_monsterID; }
	const uint16 GetMonsterType() const { return m_monsterType; }
	const uint16 GetSectorIdx() const { return m_sectorIdx; }
	const uint16 GetHP() const { return m_hp; }
	const uint16 GetMaxHP() const { return m_maxHP; }
	const EMonsterState GetMonsterState() const { return m_state; }
	const SectorPos& GetSectorPos() const { return m_secPos; }
	const Location& GetLocation() const { return m_location; }
	const Location& GetSpawnLocation() const { return m_spawnLocation; }
	const float     GetX() const { return m_location.xpos; }
	const float     GetY() const { return m_location.ypos; }
	const float     GetZ() const { return m_location.zpos; }
	const float     GetMoveYaw() const { return m_moveYaw; }
	const bool      GetMoveFlag() const { return m_moveFlag; }

	void SetSectorIdx(uint16 idx) { m_sectorIdx = idx; }
	void SetHP(uint16 hp) { m_hp = hp; }
	void SetLocation(const Location& location) { m_location = location; }

private:
	uint64        m_monsterID      = 0;
	uint16        m_monsterType    = 0;     // 0 : Khaimera
	uint16        m_sectorIdx      = 0;
	uint16        m_hp             = 50;
	uint16        m_maxHP          = 50;

	float         m_patrolRadius   = 500.0f;
	float         m_leashRange     = 1500.0f;

	float         m_moveYaw        = 0.f;
	float         m_moveSpeed      = 0.f;
	bool          m_moveFlag       = false;

	EMonsterState m_state;                  // Monster State
	SectorPos     m_secPos;     		    
	Location      m_location;               // Current Position
	Location      m_spawnLocation;

	uint32        m_respawnDelay   = 10000;
	uint32        m_respawnTime    = 0;

};


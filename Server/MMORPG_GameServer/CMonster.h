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

	uint64 GetMonsterID() const { m_monsterID; }
	uint16 GetMonsterType() const { m_monsterType; }
	uint16 GetSectorIdx() const { m_sectorIdx; }
	uint16 GetHP() const { m_hp; }
	uint16 GetMaxHP() const { m_maxHP; }
	EMonsterState GetMonsterState() const { m_state; }
	SectorPos& GetSectorPos() const { m_secPos; }
	Location& GetLocation() const { m_location; }
	Location& GetSpawnLocation() const { m_spawnLocation; }
	float     GetX() const { m_location.xpos; }
	float     GetY() const { m_location.ypos; }
	float     GetZ() const { m_location.zpos; }
	float     GetMoveYaw() const { m_moveYaw; }
	bool      GetMoveFlag() const { m_moveFlag; }

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


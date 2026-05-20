#pragma once

#include "ContentsType.h"
#include "SectorPos.h"
#include "ContentsEnum.h"
#include "ContentsStruct.h"
#include "ContentsDefine.h"

struct GrossField_MonsterSpawnInfo
{
	Location  m_spawnLocation;
	SectorPos m_spawnSectorPos;

	uint32    m_respawnDelay        = 10000;
	uint32    m_respawnTime         = 0;
						         
	float     m_patrolRadius         = 500.0f;
	float     m_leashRange           = 1500.0f;
};


class CMonster
{
public:
	CMonster() = default;
	~CMonster() = default;

	void Init(uint64 monsterID, uint32 spawnIndex , uint16 monsterType, const Location& spawnLocation);
	void Destroy();

private:
	uint64        m_monsterID      = 0;
	uint32        m_spawnIndex     = -1;    // MonsterSpawnInfo 
	uint16        m_monsterType    = 0;     // 0 : Khaimera
	uint16        m_hp             = 100;
	uint16        m_maxHP          = 100;
		          
	float         m_moveYaw        = 0.f;
	float         m_moveSpeed      = 0.f;
	bool          m_moveFlag       = false;

	EMonsterState m_state;                  // Monster State
	SectorPos     m_secPos;     		    
	Location      m_location;               // Current Position
};


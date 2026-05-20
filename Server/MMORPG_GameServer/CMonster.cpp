#include "CMonster.h"

void CMonster::Init(uint64 monsterID,  uint16 monsterType, const Location& spawnLocation)
{
	m_monsterID = monsterID;
	m_monsterType = monsterType;
	m_location = spawnLocation;
	m_moveYaw = rand() / 360;
	m_spawnLocation = spawnLocation;
	m_moveSpeed = MonsterConst::WALK_SPEED;
	m_state = EMonsterState::Idle;
	m_secPos.SetPos(SectorPos((m_location.xpos - FieldConst::MAP_WORLD_OFFSET_X) / FieldConst::SECTOR_SIZE, (m_location.ypos - FieldConst::MAP_WORLD_OFFSET_Y) / FieldConst::SECTOR_SIZE));
}

void CMonster::Regen()
{
	m_hp = MonsterConst::BASE_HP;
	m_moveSpeed = MonsterConst::WALK_SPEED;
	m_moveYaw = 0.f;
	m_moveFlag = false;
	m_state = EMonsterState::Idle;
	m_location = m_spawnLocation;
	m_secPos.SetPos(SectorPos((m_location.xpos - FieldConst::MAP_WORLD_OFFSET_X) / FieldConst::SECTOR_SIZE, (m_location.ypos - FieldConst::MAP_WORLD_OFFSET_Y) / FieldConst::SECTOR_SIZE));
}

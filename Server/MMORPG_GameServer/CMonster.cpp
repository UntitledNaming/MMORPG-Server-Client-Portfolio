#include "MonsterAI.h"
#include "CMonster.h"

void CMonster::Init(uint64 monsterID,  uint16 monsterType, const Location& spawnLocation, FieldGroup* fieldGroupPtr)
{
	m_monsterID = monsterID;
	m_monsterType = monsterType;
	m_location = spawnLocation;
	m_moveYaw = rand() % 360;
	m_state = EMonsterState::Idle;
	m_secPos.SetPos(SectorPos((m_location.xpos - FieldConst::MAP_WORLD_OFFSET_X) / FieldConst::SECTOR_SIZE, (m_location.ypos - FieldConst::MAP_WORLD_OFFSET_Y) / FieldConst::SECTOR_SIZE));

	// AI의 초기화
	m_pMonsterAIComp = new MonsterAI;
	m_pMonsterAIComp->Init(this, fieldGroupPtr, spawnLocation);
}

void CMonster::Destroy()
{
	delete m_pMonsterAIComp;
	m_pMonsterAIComp = nullptr;
}

void CMonster::Regen()
{
	m_hp = MonsterConst::BASE_HP;
	m_moveYaw = rand() % 360;
	m_state = EMonsterState::Idle;
	m_secPos.SetPos(SectorPos((m_location.xpos - FieldConst::MAP_WORLD_OFFSET_X) / FieldConst::SECTOR_SIZE, (m_location.ypos - FieldConst::MAP_WORLD_OFFSET_Y) / FieldConst::SECTOR_SIZE));

	m_pMonsterAIComp->Reset();
}

void CMonster::Move()
{
	float rad = m_moveYaw * FieldConst::Pi / 180.0f;
	float dirX = cosf(rad);
	float dirY = sinf(rad);

	m_location.xpos += dirX * m_moveSpeed;
	m_location.ypos += dirY * m_moveSpeed;

}

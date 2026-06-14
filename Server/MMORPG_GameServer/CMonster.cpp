#include <windows.h>
#include <array>
#include <unordered_map>
#include <stack>
#include <unordered_set>
#include "Inventory.h"
#include "Equipment.h"
#include "QuickSlot.h"
#include "CUserItemStorage.h"
#include "MonsterAI.h"
#include "IUser.h"
#include "CUser.h"
#include "CMonster.h"

void CMonster::Init(uint64 monsterID,  uint16 monsterType, const Location& spawnLocation, FieldGroup* fieldGroupPtr)
{
	m_monsterID = monsterID;
	m_monsterType = monsterType;
	m_location = spawnLocation;
	m_spawnLocation = spawnLocation;
	m_moveYaw = rand() % 360;
	m_state = EMonsterState::Idle;
	m_secPos.SetPos(SectorPos((m_location.xpos - FieldConst::MAP_WORLD_OFFSET_X) / FieldConst::SECTOR_SIZE, (m_location.ypos - FieldConst::MAP_WORLD_OFFSET_Y) / FieldConst::SECTOR_SIZE));
	m_atk = MonsterConst::BASE_ATK;
	m_def = MonsterConst::BASE_DEF;
	m_hp = MonsterConst::BASE_HP;
	m_maxHP = MonsterConst::BASE_MAXHP;
	m_exp = MonsterConst::GET_EXP;

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
	m_maxHP = MonsterConst::BASE_MAXHP;
	m_moveYaw = rand() % 360;
	m_location = m_spawnLocation;
	m_state = EMonsterState::Idle;
	m_secPos.SetPos(SectorPos((m_location.xpos - FieldConst::MAP_WORLD_OFFSET_X) / FieldConst::SECTOR_SIZE, (m_location.ypos - FieldConst::MAP_WORLD_OFFSET_Y) / FieldConst::SECTOR_SIZE));
	m_atk = MonsterConst::BASE_ATK;
	m_def = MonsterConst::BASE_DEF;
	m_respawnTime = 0;
	m_pMonsterAIComp->Reset();

}

void CMonster::Move()
{
	if (m_state == EMonsterState::Dead)
		return;

	float rad = m_moveYaw * FieldConst::Pi / 180.0f;
	float dirX = cosf(rad);
	float dirY = sinf(rad);

	m_location.xpos += dirX * m_moveSpeed;
	m_location.ypos += dirY * m_moveSpeed;

}

void CMonster::Damage(uint16 damage)
{
	if (m_state == EMonsterState::Dead)
		return;

	m_hp -= damage;
	if (m_hp <= 0)
	{
		m_hp = 0;
		m_state = EMonsterState::Dead;
		m_respawnTime = 0;
	}
}

void CMonster::AIUpdate()
{
	m_pMonsterAIComp->Update();
}

bool   CMonster::IsAlive()
{
	if (m_hp > 0)
		return true;

	return false;
}

uint32 CMonster::CalBaseAttackDamage(CUser* target, uint32 curTime)
{
	if (target == nullptr || !target->IsAlive())
		return 0;

	uint16 atk = GetAtk();

	float ratio = 1.0f;

	uint32 damage = static_cast<uint32>(atk * ratio);
	uint16 targetDef = target->GetDef(curTime);

	if (damage <= targetDef)
		return 1;

	return damage - targetDef;
}

Location& CMonster::GetMonsterAITargetLocation() const
{
	return m_pMonsterAIComp->GetTargetLocation();
}

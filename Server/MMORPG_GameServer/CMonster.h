#pragma once

#include "ContentsType.h"
#include "SectorPos.h"
#include "ContentsEnum.h"
#include "ContentsStruct.h"
#include "ContentsDefine.h"

class MonsterAI;
class FieldGroup;
class CUser;

class CMonster
{
public:
	CMonster() = default;
	~CMonster() = default;

	void Init(uint64 monsterID, uint16 monsterType, const Location& spawnLocation, FieldGroup* fieldGroupPtr);
	void Destroy();
	void Regen();
	void Move();
	void Damage(uint16 damage);
	uint32 CalBaseAttackDamage(CUser* target, uint32 curTime);


	Location& GetMonsterAITargetLocation() const;

    uint64 GetMonsterID() const { return m_monsterID; }
	uint32 GetRespawnTime() const { return m_respawnTime; }
	uint32 GetRespawnDelay() const { return m_respawnDelay; }
    uint16 GetMonsterType() const { return m_monsterType; }
    uint16 GetSectorIdx() const { return m_sectorIdx; }
    uint16 GetHP() const { return m_hp; }
    uint16 GetMaxHP() const { return m_maxHP; }
	uint16 GetSectorX() const { return m_secPos.GetX(); }
	uint16 GetSectorY() const { return m_secPos.GetY(); }
	uint16 GetAtk() const { return m_atk; }
	uint16 GetDef() const { return m_def; }
	EMonsterState GetMonsterState() const { return m_state; }
	const SectorPos& GetSectorPos() const { return m_secPos; }
	const Location& GetLocation() const { return m_location; }
	float     GetX() const { return m_location.xpos; }
	float     GetY() const { return m_location.ypos; }
	float     GetZ() const { return m_location.zpos; }
	float     GetMoveYaw() const { return m_moveYaw; }
	float     GetMoveSpeed() const { return m_moveSpeed; }

	void SetMoveYaw(float moveYaw) { m_moveYaw = moveYaw; }
	void SetSectorIdx(uint16 idx) { m_sectorIdx = idx; }
	void SetHP(uint16 hp) { m_hp = hp; }
	void SetLocation(const Location& location) { m_location = location; }
	void SetMoveSpeed(float moveSpeed) { m_moveSpeed = moveSpeed; }
	void ChangeMonsterState(EMonsterState state) { m_state = state; }
	void IncRespawnTime() { m_respawnTime += FieldConst::UPDATE_LOOP_TIME; }

private:
	uint64        m_monsterID      = 0;
	uint16        m_monsterType    = 0;     // 0 : Khaimera
	uint16        m_sectorIdx      = 0;
	uint16        m_hp             = 50;
	uint16        m_maxHP          = 50;
	uint16        m_atk            = 5;
	uint16        m_def            = 1;
	float         m_moveYaw        = 0.f;
	float         m_moveSpeed      = 0.f;

	EMonsterState m_state;                  // Monster State
	SectorPos     m_secPos;     		    
	Location      m_location;               // Current Position

	uint32        m_respawnDelay   = 10000;
	uint32        m_respawnTime    = 0;

	MonsterAI*    m_pMonsterAIComp = nullptr;
};


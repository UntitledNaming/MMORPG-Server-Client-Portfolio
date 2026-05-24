#pragma once
#include "ContentsType.h"
#include "ContentsStruct.h"
#include "ContentsEnum.h"

class CMonster;
class FieldGroup;
class CUser;

class MonsterAI
{
public:
	void Init(CMonster* pOwner, FieldGroup* pField, const Location& spawnLocation);
	void Update();
	void Reset();
	Location& GetTargetLocation() { return m_targetLocation; }

private:
	void UpdateIdle();
	void UpdatePatrol();
	void UpdateChase();
	void UpdateReturn();
	void UpdateCombat();

	void EnterPatrol();
	void EnterChase(CUser* targetPlayer);
	void EnterReturn();
	void EnterIdle();
	void EnterCombat();

	CUser* FindNearestPlayer(float range);
	void UpdateSector();
	void TargetUpdate();
	bool IsNear(const Location& cur, const Location& target);
	bool IsAttackRange();

private:
	CMonster*   m_pOwner = nullptr;
	CUser*      m_pTarget = nullptr;
	FieldGroup* m_pField = nullptr;

	Location    m_spawnLocation = {};
	Location    m_targetLocation = {};

	uint32      m_idleElapsed = 0;            // idle 대기 시간 재기 위한 변수
	uint32      m_idleDuration = 0;      

	bool        m_patrolPausing = false;
	uint32      m_pauseElapsed = 0;
			    
	uint32      m_chaseUpdateAccum = 0;       // 추격 시 누적시간

	uint32      m_attackAccum = 0;            // 공격 시 누적시간
			    						      
};


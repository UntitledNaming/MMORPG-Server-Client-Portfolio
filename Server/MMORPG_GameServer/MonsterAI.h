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

private:
	void UpdateIdle();
	void UpdatePatrol();
	void UpdateChase();
	void UpdateReturn();

	void EnterPatrol();
	void EnterChase(uint64 targetSessionID);
	void EnterReturn();
	void EnterIdle();

	CUser* FindNearestPlayer(float range);
	void UpdateSector();
	bool IsNear(const Location& cur, const Location& target);

private:
	CMonster*   m_pOwner = nullptr;
	FieldGroup* m_pField = nullptr;

	Location    m_spawnLocation = {};
	Location    m_targetLocation = {};

	uint32      m_idleElapsed = 0;            // idle 대기 시간 재기 위한 변수
	uint32      m_idleDuration = 0;      

	bool        m_patrolPausing = false;
	uint32      m_pauseElapsed = 0;
			    
	uint64      m_chaseTargetID = 0;
	float       m_lastChaseYaw = 0.f;
	uint32      m_chaseUpdateAccum = 0;       // 추격 시 누적시간
	uint32      m_attackAccum = 0;            // 공격 시 누적시간
			    						      
	uint32      m_syncAccum = 0;              // 위치 싱크 맞출 때 사용할 누적시간
};


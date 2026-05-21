#include <cstdlib>
#include <cmath>
#include "ContentsDefine.h"
#include "SectorPos.h"
#include "FieldSector.h"
#include "IUser.h"
#include "CUser.h"
#include "CMonster.h"
#include "CGroup.h"
#include "FieldGroup.h"
#include "MonsterAI.h"

using namespace MonsterConst;
using namespace FieldConst;

void MonsterAI::Init(CMonster* pOwner, FieldGroup* pField, const Location& spawnLocation)
{
	m_pOwner = pOwner;
	m_pField = pField;
	m_idleDuration = IDLE_MIN_DURATION_MS + rand() % (IDLE_MAX_DURATION_MS - IDLE_MIN_DURATION_MS);
	m_spawnLocation = spawnLocation;
	EnterIdle();
}

void MonsterAI::Update()
{
	switch (m_pOwner->GetMonsterState())
	{
	case EMonsterState::Idle :
		UpdateIdle();
		break;

	case EMonsterState::Patrol:
		UpdatePatrol();
		break;

	case EMonsterState::Chase:
		UpdateChase();
		break;

	case EMonsterState::Return:
		UpdateReturn();
		break;

	case EMonsterState::Dead:
		break;
	}
}

void MonsterAI::Reset()
{
	m_targetLocation = {};
	m_patrolPausing = false;
	m_pauseElapsed = 0;
	m_chaseTargetID = 0;
	m_lastChaseYaw = 0.f;
	m_chaseUpdateAccum = 0;
	m_attackAccum = 0;
	m_syncAccum = 0;

	EnterIdle();
}

void MonsterAI::UpdateIdle()
{
	CUser* pTarget = FindNearestPlayer(DETECT_RANGE);
	if (pTarget)
	{
		EnterChase(pTarget->GetSessionID());
		return;
	}

	m_idleElapsed += UPDATE_LOOP_TIME;
	if (m_idleElapsed >= m_idleDuration)
		EnterPatrol();
}

void MonsterAI::UpdatePatrol()
{
	CUser* pTarget = FindNearestPlayer(DETECT_RANGE);
	if (pTarget)
	{
		EnterChase(pTarget->GetSessionID());
		return;
	}

	// 타겟 없고 목적지 도착후 잠깐 정지한 상태가 아닌 경우 목적지로 이동
	if (!m_patrolPausing)
	{
		// 몬스터 이동 시키기
		m_pOwner->Move();


		// 스폰 위치 근처로 왔으면 Idle 상태로 진입
		// 목적지 근처인 경우 정지 플래그 키고 시간 재기 시작
		// 목적지와의 거리가 몬스터의 한틱 거리보다 이하면 도착으로 판단
		if (IsNear(m_pOwner->GetLocation(), m_targetLocation))
		{
			m_patrolPausing = true;
			m_pauseElapsed = 0;
			m_pOwner->SetLocation(m_targetLocation);
		}

		return;
	}

	// 정지 상태인 경우 정지 대기 시간 지났는지 판단
	m_pauseElapsed += UPDATE_LOOP_TIME;

	// 만약 시간이 안 지났으면 함수 리턴
	if (m_pauseElapsed < PATROL_PAUSE_MS)
		return;

	// 시간 지났으면 리턴 상태로 돌입.
	m_patrolPausing = false;
	EnterReturn();
}

void MonsterAI::UpdateChase()
{

}

void MonsterAI::UpdateReturn()
{
	// 리턴 중에 타겟 발견되면 추격
	CUser* pTarget = FindNearestPlayer(DETECT_RANGE);
	if (pTarget)
	{
		EnterChase(pTarget->GetSessionID());
		return;
	}

	// 타겟 발견이 안되었으면 스폰 위치로 이동
	m_pOwner->Move();

	// 스폰 위치 근처로 왔으면
	if (IsNear(m_pOwner->GetLocation(), m_spawnLocation))
	{
		m_pOwner->SetLocation(m_spawnLocation);
		EnterIdle();
	}
}

void MonsterAI::EnterPatrol()
{
	m_pOwner->ChangeMonsterState(EMonsterState::Patrol);
	m_pOwner->SetMoveSpeed(PATROL_SPEED / UPDATE_LOOP_TIME);

	// 랜덤으로 방향 설정
	float yaw = rand() % 360;
	m_pOwner->SetMoveYaw(yaw);

	float rad = yaw * FieldConst::Pi / 180.0f;

	// 해당 방향으로 range 위치에 있는 patrolTarget 설정
	m_targetLocation.xpos = m_spawnLocation.xpos + cosf(rad) * PATROL_DISTANCE;
	m_targetLocation.ypos = m_spawnLocation.ypos + sinf(rad) * PATROL_DISTANCE;
	m_targetLocation.zpos = m_spawnLocation.zpos;

}

void MonsterAI::EnterChase(uint64 targetSessionID)
{
	m_pOwner->ChangeMonsterState(EMonsterState::Chase);
	m_pOwner->SetMoveSpeed(CHASE_SPEED / UPDATE_LOOP_TIME);

}

void MonsterAI::EnterReturn()
{
	// 리턴 상태로 변경
	m_pOwner->ChangeMonsterState(EMonsterState::Return);
	m_pOwner->SetMoveSpeed(RETURN_SPEED / UPDATE_LOOP_TIME);

	// 현재 위치에서 스폰 위치 방향을 구해서 해당 방향을 moveYaw로 변경
	float dx = m_spawnLocation.xpos - m_pOwner->GetX();
	float dy = m_spawnLocation.ypos - m_pOwner->GetY();
	float rad = atan2f(dy, dx);

	m_pOwner->SetMoveYaw(rad * 180.0f / FieldConst::Pi);
}

void MonsterAI::EnterIdle()
{
	m_pOwner->ChangeMonsterState(EMonsterState::Idle);
	m_pOwner->SetMoveSpeed(0);
	m_idleElapsed = 0;
}

CUser* MonsterAI::FindNearestPlayer(float range)
{
	return nullptr;
}

void MonsterAI::UpdateSector()
{
}

bool MonsterAI::IsNear(const Location& cur, const Location& target)
{
	float dx = std::abs(cur.xpos - target.xpos);
	float dy = std::abs(cur.xpos - target.ypos);
	float distSq = dx * dx + dy * dy;

	// 목적지와의 거리가 몬스터의 한틱 거리보다 이하면 도착으로 판단
	float arriveMinDistSq = m_pOwner->GetMoveSpeed() * m_pOwner->GetMoveSpeed();

	if (distSq <= arriveMinDistSq)
		return true;

	return false;
}


#include <cstdlib>
#include <cmath>
#include <limits>
#include <vector>
#include "ContentsDefine.h"
#include "SectorPos.h"
#include "IUser.h"
#include "CUser.h"
#include "CMonster.h"
#include "PacketBuilder.h"
#include "CGroup.h"
#include "FieldSector.h"
#include "FieldGroup.h"
#include "CollisionCheck.h"
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
	m_pTarget = nullptr;
	m_targetLocation = {};
	m_patrolPausing = false;
	m_pauseElapsed = 0;
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
		EnterChase(pTarget);
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
		EnterChase(pTarget);
		return;
	}

	// 타겟 없고 목적지 도착후 잠깐 정지한 상태가 아닌 경우 목적지로 이동
	if (!m_patrolPausing)
	{
		// 몬스터 이동 시키기
		m_pOwner->Move();

		UpdateSector();

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
	// 타겟 캐릭터가 죽었거나 연결이 끊어진 경우 리턴
	if (!m_pTarget->IsAlive())
	{
		EnterReturn();
		return;
	}

	// 스폰 위치와 거리가 멀어지면 Return
	float dx = m_spawnLocation.xpos - m_pOwner->GetX();
	float dy = m_spawnLocation.ypos - m_pOwner->GetY();
	float distSq = dx * dx + dy * dy;

	if (distSq >= RETURN_RANGE * RETURN_RANGE)
	{
		EnterReturn();
		return;
	}

	// 타겟 좌표 업데이트(타겟과의 방향이 틀어지거나 일정 시간 지났으면)
	TargetUpdate();


	// 타겟 방향으로 이동
	m_pOwner->Move();

	// 범위 안에 들어오면 공격
	if (!IsAttackRange())
		return;

	// 타겟에게 데미지 주기
	uint32 damage = m_pOwner->CalBaseAttackDamage(m_pTarget, timeGetTime());
	m_pTarget->Damage(damage);

	// 공격 및 데미지 패킷 보내기
	m_pField->SendMonsterAttackTarget(m_pOwner, m_pTarget, m_pTarget->GetHP());
}

void MonsterAI::UpdateReturn()
{
	// 스폰 위치로 이동
	m_pOwner->Move();

	// 이동후 섹터 체크
	UpdateSector();

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

	m_patrolPausing = false;

	// 랜덤으로 방향 설정
	float yaw = rand() % 360;
	m_pOwner->SetMoveYaw(yaw);

	float rad = yaw * FieldConst::Pi / 180.0f;

	// 해당 방향으로 range 위치에 있는 patrolTarget 설정
	m_targetLocation.xpos = m_spawnLocation.xpos + cosf(rad) * PATROL_DISTANCE;
	m_targetLocation.ypos = m_spawnLocation.ypos + sinf(rad) * PATROL_DISTANCE;
	m_targetLocation.zpos = m_spawnLocation.zpos;

}

void MonsterAI::EnterChase(CUser* targetPlayer)
{
	m_pOwner->ChangeMonsterState(EMonsterState::Chase);
	m_pOwner->SetMoveSpeed(CHASE_SPEED / UPDATE_LOOP_TIME);
	m_pTarget = targetPlayer;

	// 타겟 설정
	m_targetLocation = m_pTarget->GetLocation();

	// 방향 변경
	float dx = m_targetLocation.xpos - m_pOwner->GetX();
	float dy = m_targetLocation.ypos - m_pOwner->GetY();
	float rad = atan2f(dy, dx);
	float targetYaw = rad * 180.0f / FieldConst::Pi;

	m_pOwner->SetMoveYaw(targetYaw);
	m_lastChaseYaw = targetYaw;
	m_chaseUpdateAccum = 0;
	m_attackAccum = 0;
}

void MonsterAI::EnterReturn()
{
	// 리턴 상태로 변경
	m_pOwner->ChangeMonsterState(EMonsterState::Return);
	m_pOwner->SetMoveSpeed(RETURN_SPEED / UPDATE_LOOP_TIME);

	// 타겟 설정
	m_targetLocation.xpos = m_spawnLocation.xpos;
	m_targetLocation.ypos = m_spawnLocation.ypos;
	m_targetLocation.zpos = m_spawnLocation.zpos;

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
	int minSX = (int)((m_pOwner->GetX() - range - FieldConst::MAP_WORLD_OFFSET_X) / SECTOR_SIZE);
	int maxSX = (int)((m_pOwner->GetX() + range - FieldConst::MAP_WORLD_OFFSET_X) / SECTOR_SIZE);
	int minSY = (int)((m_pOwner->GetY() - range - FieldConst::MAP_WORLD_OFFSET_Y) / SECTOR_SIZE);
	int maxSY = (int)((m_pOwner->GetY() + range - FieldConst::MAP_WORLD_OFFSET_Y) / SECTOR_SIZE);

	minSX = max(0, minSX);
	minSY = max(0, minSY);
	maxSX = min(SECTOR_X_MAX - 1, maxSX);
	maxSY = min(SECTOR_Y_MAX - 1, maxSY);

	// 탐색 범위 안에 있는 섹터에 있는 유저들 순회해서 거리가 가장 가까운 유저 찾기
	CUser* nearestUser = nullptr;
	float nearestDistSq  = (std::numeric_limits<float>::max)();

	for (int sy = minSY; sy <= maxSY; sy++)
	{
		for (int sx = minSX; sx <= maxSX; sx++)
		{
			FieldSector& sector = m_pField->GetFieldSector(sx, sy);

			int userCount = sector.GetUserCount();
			for (int i = 0; i < userCount; i++)
			{
				float dx = sector.GetUser(i)->GetX() - m_pOwner->GetX();
				float dy = sector.GetUser(i)->GetY() - m_pOwner->GetY();

				float distSq = dx * dx + dy * dy; // 거리 제곱
				float rangeSq = range * range;    // 사거리 제곱

				// 사거리 밖이면 false
				if (distSq > rangeSq)
					continue;

				// 범위 안이면 최소 거리값과 비교하여 가장 가까운지 체크
				if (distSq >= nearestDistSq)
					continue;

				nearestDistSq = distSq;
				nearestUser = sector.GetUser(i);
			}
		}
	}

	return nearestUser;
}

void MonsterAI::UpdateSector()
{
	uint16 newX = (m_pOwner->GetX() - MAP_WORLD_OFFSET_X) / SECTOR_SIZE;
	uint16 newY = (m_pOwner->GetY() - MAP_WORLD_OFFSET_Y) / SECTOR_SIZE;
	SectorPos newSec(newX, newY);

	// 범위 벗어나면 리턴
	if (!SectorPos::SectorRangeCheck(newSec))
		return;

	// 같은 섹터면 리턴
	if (SectorPos::SameSector(m_pOwner->GetSectorPos(), newSec))
		return;
	
	// 새로운 섹터면 시야에 새로운 섹터에 몬스터 생성 및 Move 패킷, 시야에 사라지는 영역에는 몬스터 삭제 패킷 보내기
	SectorAround DeleteSector;
	SectorAround CreateSector;
	SectorPos::CalSectorTransitionMessageTargets(m_pOwner->GetSectorPos(), newSec, DeleteSector, CreateSector);

	// 몬스터 삭제 메세지 보내기
	for (int i = 0; i < DeleteSector.m_count; i++)
	{
		m_pField->SendMonsterDeleteToSector(m_pOwner, DeleteSector.m_Around[i].GetX(), DeleteSector.m_Around[i].GetY());
	}

	// 몬스터 생성 메세지 보내기
	for (int i = 0; i < CreateSector.m_count; i++)
	{
		m_pField->SendMonsterCreateToSector(m_pOwner, CreateSector.m_Around[i].GetX(), CreateSector.m_Around[i].GetY());
	}

	// 기존 섹터에서 몬스터 삭제
	m_pField->RemoveMonsterToSector(m_pOwner, m_pOwner->GetSectorX(), m_pOwner->GetSectorY());

	// 새로운 섹터에 몬스터 삽입
	m_pField->AddMonsterToSector(m_pOwner, newSec.GetX(), newSec.GetY());
}

void MonsterAI::TargetUpdate()
{
	bool update = false;
	float dx = m_targetLocation.xpos - m_pOwner->GetX();
	float dy = m_targetLocation.ypos - m_pOwner->GetY();
	float rad = atan2f(dy, dx);
	float targetYaw = rad * 180.0f / FieldConst::Pi;


	// 타겟과의 방향이 크게 틀어지거나 특정 시간 지났으면 Target 위치 찾아서 업데이트
	m_chaseUpdateAccum += UPDATE_LOOP_TIME;

	if (m_chaseUpdateAccum >= CHASE_UPDATE_MIN_MS || std::abs(m_lastChaseYaw - targetYaw) >= CHASE_ANGLE_THRESHOLD)
	{
		update = true;
		m_chaseUpdateAccum = 0;
	}

	if (!update)
		return;

	// 몬스터 이동 방향 및 타겟 좌표 업데이트
	m_pOwner->SetMoveYaw(targetYaw);
	m_lastChaseYaw = targetYaw;
	m_targetLocation = m_pTarget->GetLocation();

	// Move 패킷 몬스터 주변에 뿌리기
	SectorAround MoveAround;
	SectorPos::SectorFind(MoveAround, m_pOwner->GetSectorPos());

	for (int i = 0; i < MoveAround.m_count; i++)
	{
		m_pField->SendMonsterTargetUpdateToSector(m_pOwner, m_pOwner->GetSectorX(), m_pOwner->GetSectorY());
	}
}

bool MonsterAI::IsNear(const Location& cur, const Location& target)
{
	float dx = std::abs(cur.xpos - target.xpos);
	float dy = std::abs(cur.ypos - target.ypos);
	float distSq = dx * dx + dy * dy;

	// 목적지와의 거리가 몬스터의 한틱 거리보다 이하면 도착으로 판단
	float arriveMinDistSq = m_pOwner->GetMoveSpeed() * m_pOwner->GetMoveSpeed();

	if (distSq <= arriveMinDistSq)
		return true;

	return false;
}

bool MonsterAI::IsAttackRange()
{
	if (!CollisionCheck::IsInCone(m_pOwner->GetLocation(), m_pTarget->GetLocation(), ATTACK_RANGE, m_pOwner->GetMoveYaw(), ATTACK_HALF_ANGLE))
		return false;

	return true;
}


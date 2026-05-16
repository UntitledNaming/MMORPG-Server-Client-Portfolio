#include <string>
#include <windows.h>
#include "ContentsDefine.h"
#include "ContentsEnum.h"
#include "MemoryPoolTLS.h"
#include "IUser.h"
#include "CUser.h"

CMPoolTLS<CUser> CUser::m_userPool;

using namespace UserConst;


void CUser::Init(uint64 sessionID)
{
	// todo : 추후 DB에서 데이터 긁어와서 초기화 하기
	// 
	// movespeed는 실질적으로 max walk speed임. 이벤트 방식을 통해 특정 이벤트 발생시 max walk speed가 변경되면 
	// 이를 클라에게 전파해서 해당 클라의 max walk speed를 변경하여 이동 속도를 조절함.
	// 캐릭터 생성, 삭제시 이 Max Walk Speed를 서버가 전파해주던가 아니면 프로토콜로 버프, 디버프 상태를
	// 추가하여 이를 전달해 클라가 자체 계산해서 max speed값 조정하던가 해야 함.

	m_sessionID = sessionID;
	m_xpos = 405430.0f;
	m_ypos = 397350.0f;
	m_zpos = -38690.f;
	m_isFalling = false;
	m_moveFlag = false;
	m_atk = 5;
	m_def = 1;
	m_hp = 100;
	m_maxHP = 100;
	m_mp = 100;
	m_maxMP = 100;
	m_mpRegenPerSec = 5;
	m_sectorXpos = (m_xpos - FieldConst::MAP_WORLD_OFFSET_X) / FieldConst::SECTOR_SIZE;
	m_sectorYpos = (m_ypos - FieldConst::MAP_WORLD_OFFSET_Y) / FieldConst::SECTOR_SIZE;
	m_syncCount = 0;
	m_movementYaw = 0.0f;
	m_maxWalkSpeed = WALK_SPEED;
	m_moveSpeed = m_maxWalkSpeed / FieldConst::UPDATE_FRAME;
	m_recvTime = timeGetTime();
	m_lastSyncCheckTime = timeGetTime();

	memcpy_s(m_nickName, NICK_MAX, L"MY", NICK_MAX);

	// SwingInfo 초기화
	m_swingInfo.m_lastSwingIdx = 0;
	m_swingInfo.m_lastSwingRecvTime = 0;

	// SkillInfo 초기화
	for (int i = 0; i < USER_SKILL_SLOT_COUNT; i++)
	{
		m_skillInfo[i].m_skillActivate = false;
		m_skillInfo[i].m_skillLastRecvTime = 0;
		m_skillInfo[i].m_skillExpiredTime = 0;
	}
}

void CUser::SkillInfoUpdate(uint16 skillIndex, uint32 curTime, bool bActivate)
{
	if (skillIndex >= USER_SKILL_SLOT_COUNT)
		return;

	m_skillInfo[skillIndex].m_skillActivate = bActivate;
	m_skillInfo[skillIndex].m_skillLastRecvTime = timeGetTime();

	switch (skillIndex)
	{
	case 0:

		DefUpdate();

		m_skillInfo[skillIndex].m_skillExpiredTime = curTime + ClientAttack::DEFENCE_BUFF_DURATION;
		m_mp -= ClientAttack::DEFENCE_BUFF_REQUIRED_MANA;
		m_def += ClientAttack::DEFENCE_BUFF_ADD_AMOUNT;
		break;

	case 1:
		break;

	case 2:
		break;

	case 3:
		break;
	}

	m_skillInfo[skillIndex].m_skillActivate = bActivate;
	m_skillInfo[skillIndex].m_skillLastRecvTime = timeGetTime();
}

uint16 CUser::GetDef()
{
	DefUpdate();

	return m_def;
}

CUser* CUser::Alloc()
{
	return m_userPool.Alloc();
}


void CUser::Free(CUser* pUser)
{
	m_userPool.Free(pUser);
}

void CUser::DefUpdate()
{
	// 자 버프 만료 시간 체크
	SkillInfo& defenceBuff = m_skillInfo[0];

	if (defenceBuff.m_skillActivate && timeGetTime() >= defenceBuff.m_skillExpiredTime)
	{
		m_def -= ClientAttack::DEFENCE_BUFF_ADD_AMOUNT;
		defenceBuff.m_skillActivate = false;
	}

	// 아이템 버프 체크

	// 타 버프 만료 시간 체크
}

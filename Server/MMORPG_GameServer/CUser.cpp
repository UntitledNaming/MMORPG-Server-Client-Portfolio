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
	m_mp = 50;
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
}

CUser* CUser::Alloc()
{
	return m_userPool.Alloc();
}


void CUser::Free(CUser* pUser)
{
	m_userPool.Free(pUser);
}

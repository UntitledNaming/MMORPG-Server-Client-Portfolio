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
	m_sessionID = sessionID;
	m_xpos = 405430;
	m_ypos = 397350;
	m_zpos = -38754.f;
	m_isFalling = false;
	m_moveFlag = false;
	m_hp = 100;
	m_maxHP = 100;
	m_mp = 100;
	m_maxMP = 100;
	m_sectorXpos = m_xpos / FieldConst::SECTOR_SIZE;
	m_sectorYpos = m_ypos / FieldConst::SECTOR_SIZE;
	m_action = EM1ActionStateType::None;
	m_moveMode = EM1MoveMode::None;
	m_movementYaw = 0.0f;
	m_moveSpeed = 0;
	m_recvTime = timeGetTime();

	memcpy_s(m_nickName, NICK_MAX, L"MY", NICK_MAX);
}

CUser* CUser::Alloc()
{
	return m_userPool.Alloc();
}


void CUser::Free(CUser* pUser)
{
	m_userPool.Free(pUser);
}

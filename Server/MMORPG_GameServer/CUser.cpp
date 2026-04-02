#include <string>
#include <windows.h>
#include "ContentsDefine.h"
#include "MemoryPoolTLS.h"
#include "IUser.h"
#include "CUser.h"

CMPoolTLS<CUser> CUser::m_userPool;

using namespace UserConst;


void CUser::Init(UINT64 sessionID)
{
	m_sessionID = sessionID;
	m_xpos = 0;
	m_ypos = 0;
	m_hp = 100;
	m_mp = 100;
	m_sectorXpos = m_xpos / FieldConst::SECTOR_SIZE;
	m_sectorYpos = m_ypos / FieldConst::SECTOR_SIZE;
	m_action = CUser::USER_ACTION::STOP;
	m_inputMask = InputMask::None;
	m_cameraYaw = 0;
	m_walkSpeed = WALK_SPEED;
	m_runSpeed = RUN_SPEED;
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

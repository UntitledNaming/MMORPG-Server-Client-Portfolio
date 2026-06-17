#include <windows.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <array>
#include <set>
#include "ContentsEnum.h"
#include "ContentsDefine.h"
#include "ContentsProtocol.h"
#include "MemoryPoolTLS.h"
#include "CMessage.h"
#include "Inventory.h"
#include "Equipment.h"
#include "QuickSlot.h"
#include "CUserItemStorage.h"
#include "IUser.h"
#include "CUser.h"
#include "CGroup.h"
#include "AuthGroup.h"

using namespace AuthConst;
using namespace AuthProtocol;

void AuthGroup::Init(CGameLibrary* p)
{
	m_pGameLib = p;
	m_GroupFrameTime = NONUSER_TIMEOUT;
	m_OldTime = timeGetTime();
	m_Shared = false;
	m_RecvTPS = 0;
	m_SendTPS = 0;
	m_FrameTPS = 0;
	InitializeSRWLock(&m_GroupLock);
}

void AuthGroup::Destroy()
{

}

void AuthGroup::OnClientJoin(uint64 sessionID)
{
	m_nonuserTable.insert(std::pair<uint64, DWORD>(sessionID, timeGetTime()));
}

void AuthGroup::OnClientLeave(uint64 sessionID)
{
	// NonUserMap에서 찾기
	std::unordered_map<uint64, DWORD>::iterator it = m_nonuserTable.find(sessionID);
	if (it != m_nonuserTable.end())
	{
		// 제거
		m_nonuserTable.erase(it);
		return;
	}

	// NonUserMap에 없으면 UserMap에서 찾기
	std::unordered_map<uint64, CUser*>::iterator it2 = m_userTable.find(sessionID);
	if (it2 == m_userTable.end())
		__debugbreak();

	CUser* pUser = it2->second;
	m_userTable.erase(it2);

	CUser::Free(pUser);
}

void AuthGroup::OnRecv(uint64 sessionID, CMessage* pMessage)
{
	uint16 type;
	*pMessage >> type;

	switch (type)
	{
	case PACKET_CS_GAME_LOGIN_REQ:
		LoginRequestProc(sessionID, pMessage);
		break;

	case PACKET_CS_GAME_CHARACTER_SELECT:
		CharacterSelectProc(sessionID, pMessage);
		break;
	}
}

void AuthGroup::OnIUserMove(uint64 sessionID, IUser* pUser)
{

}

void AuthGroup::OnUpdate()
{
	// NonUserTimeOut

	// UserTimeOut

	// DBManager가 전달한 유저 정보 꺼내서 세팅 후 FieldGroup으로 전송
}

void AuthGroup::LoginRequestProc(uint64 sessionID, CMessage* pMessage)
{
	m_nonuserTable.erase(sessionID);

	CUser* pUser = CUser::Alloc();
	pUser->Init(sessionID);

	// todo : Redis에 토큰 조회

	// todo : 인증 실패면 연결 끊기

	// todo : 성공이 유저 객체에 저장

	// todo : 성공시 DB에서 Redis에서 가져온 AccountID에 해당하는 해당 계정이 소유한 캐릭터 이름, 레벨, 외형정보 등을 가져와 클라에게 Send

	m_userTable.insert(std::pair<uint64, CUser*>(sessionID, pUser));
}

void AuthGroup::CharacterSelectProc(uint64 sessionID, CMessage* pMessage)
{
	uint64 characteruid;
	*pMessage >> characteruid;

	// characterUID에 대한 검증.

	// 해당 UID에 해당하는 캐릭터 정보 얻는 Job DBManager에게 전달

}

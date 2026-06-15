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

void AuthGroup::OnClientJoin(UINT64 sessionID)
{
	m_nonuserTable.insert(std::pair<uint64, DWORD>(sessionID, timeGetTime()));
}

void AuthGroup::OnClientLeave(UINT64 sessionID)
{
	m_nonuserTable.erase(sessionID);
}

void AuthGroup::OnRecv(UINT64 sessionID, CMessage* pMessage)
{
	uint16 type;
	*pMessage >> type;

	switch (type)
	{
	case PACKET_CS_GAME_LOGIN_REQ:
		LoginRequestProc(sessionID, pMessage);
		break;

	}
}

void AuthGroup::OnIUserMove(UINT64 sessionID, IUser* pUser)
{
}

void AuthGroup::OnUpdate()
{
	// NonUserTimeOut
}

void AuthGroup::LoginRequestProc(UINT64 sessionID, CMessage* pMessage)
{
	std::wstring field = L"Field";
	m_nonuserTable.erase(sessionID);

	CUser* pUser = CUser::Alloc();
	pUser->Init(sessionID);

	GroupMove(field, sessionID, (IUser*)pUser);
}

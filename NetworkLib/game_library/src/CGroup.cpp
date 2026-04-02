#include <windows.h>
#include <unordered_map>
#include <string>
#include <thread>

#include "MemoryPoolTLS.h"
#include "CMessage.h"
#include "IUser.h"

#include "LFStack.h"
#include "LFQSingleLive.h"

#include "Ring_Buffer.h"
#include "LFQMultiLive.h"
#include "GameSession.h"
#include "CGameLibrary.h"
#include "CGroup.h"


bool    CGroup::SendPacket(UINT64 SessionID, CMessage* pMessage)
{
	return m_pGameLib->SendPacket(SessionID, pMessage);
}

bool    CGroup::Disconnect(UINT64 SessionID)
{
	return m_pGameLib->Disconnect(SessionID);
}

bool    CGroup::FindIP(UINT64 SessionID, std::wstring& OutIP)
{
	return m_pGameLib->FindIP(SessionID, OutIP);
}

bool    CGroup::GroupMove(std::wstring& ToContents, UINT64 sessionID, IUser* pUser)
{
	return m_pGameLib->GroupMove(ToContents, sessionID, pUser);
}

CGroup* CGroup::GetGroupPtr(std::wstring& Contents)
{
	return m_pGameLib->GetGroupPtr(Contents);
}
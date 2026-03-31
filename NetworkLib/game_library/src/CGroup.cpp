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

LONG    CGroup::GetAcceptTPS()
{
	return m_pGameLib->GetAcceptTPS();
}

LONG    CGroup::GetRecvIOTPS()
{
	return m_pGameLib->GetRecvIOTPS();
}

LONG    CGroup::GetSendIOTPS()
{
	return m_pGameLib->GetSendIOTPS();
}

INT64   CGroup::GetAcceptTotal()
{
	return m_pGameLib->GetAcceptTotal();
}

SHORT   CGroup::GetCurSessionCount()
{
	return m_pGameLib->GetCurSessionCount();
}

void    CGroup::SetAcceptTPS(LONG value)
{
	return m_pGameLib->SetAcceptTPS(value);
}

void    CGroup::SetRecvIOTPS(LONG value)
{
	return m_pGameLib->SetRecvIOTPS(value);
}

void    CGroup::SetSendIOTPS(LONG value)
{
	return m_pGameLib->SetSendIOTPS(value);
}

CGroup* CGroup::GetGroupPtr(std::wstring& Contents)
{
	return m_pGameLib->GetGroupPtr(Contents);
}
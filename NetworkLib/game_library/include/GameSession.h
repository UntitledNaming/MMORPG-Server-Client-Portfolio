#pragma once
#include "GameLibDefine.h"

using namespace GameSession;

class CSession
{

public:
	CSession();
	~CSession();

	void Init(SOCKET socket, UINT64 sessionID);

public:
	SOCKET                 m_Socket;
	UINT64                 m_SessionID;
	OVERLAPPED             m_RecvOverlapped;
	OVERLAPPED             m_SendOverlapped;
	CRingBuffer            m_RecvQ;
	LFQueueMul<CMessage*>  m_SendQ;
	CMessage*              m_SendArray[GAMESESSION_WSABUFSIZE];
	UINT16                 m_GroupID;
	SHORT                  m_SendFlag;
	INT                    m_SendMsgCnt;
	LONG                   m_DCFlag;
	alignas(16)LONGLONG    m_RefCnt;
	LONGLONG               m_RelFlag;

};




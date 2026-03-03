#include <windows.h>
#include <string>
#include <thread>
#include <unordered_map>
#include <Pdh.h>
#include <codecvt>
#include <mysql.h>
#include "CommonProtocol.h"
#include "LibraryHeader.h"
#include "CUser.h"
#include "CPUUsage.h"
#include "ProcessMonitor.h"
#include "LogClass.h"
#include "TextParser.h"
#include "Ring_Buffer.h"
#include "DBTLS.h"
#include "myList.h"
#include "LockFreeMemoryPoolLive.h"
#include "MemoryPoolTLS.h"
#include "LFQSingleLive.h"
#include "LFQMultiLive.h"
#include "LFStack.h"
#include "CMessage.h"
#include "CSession.h"
#include "ServerContext.h"
#include "IModule.h"
#include "CLanServer.h"
#include "GameServer.h"

GameServer::GameServer() : m_moduleTBLIdx(0), m_endflag(false)
{

}

GameServer::~GameServer()
{

}

void GameServer::RunServer()
{
	timeBeginPeriod(1);

	Parser parser;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

	if (!parser.LoadFile("Config.txt"))
		return;

	Parser::st_Msg bindip;
	parser.GetValue("BIND_IP", &bindip);

	std::wstring bindstr = converter.from_bytes(bindip.s_ptr);
	
	INT bindport;
	parser.GetValue("BIND_PORT", &bindport);

	Parser::st_Msg dbip;
	parser.GetValue("DB_IP", &dbip);


	INT dbport;
	parser.GetValue("DB_PORT", &dbport);


	INT maxSessions;
	parser.GetValue("SESSION_MAX", &maxSessions);

	INT createthread;
	parser.GetValue("IOCP_WORKER_THREAD", &createthread);

	INT runningthread;
	parser.GetValue("IOCP_ACTIVE_THREAD", &runningthread);

	INT Usermax;
	parser.GetValue("USER_MAX", (int*)&Usermax);

	INT Nagle;
	parser.GetValue("NAGLE", (int*)&Nagle);

	INT SendFrame;
	parser.GetValue("SEND_FRAME", (int*)&SendFrame);

	INT Sendflag;
	parser.GetValue("SEND_TH_FLAG", (int*)&Sendflag);

	INT Loglevel;
	parser.GetValue("LOG_LEVEL", (int*)&Loglevel);

	CLogClass::GetInstance()->Init(Loglevel);
	CMessage::Init(sizeof(LANHEADER), sizeof(NETHEADER));

	Mem_Init(Usermax, dbip.s_ptr, dbport);

	if (!Start(bindstr.c_str(), bindport, createthread, runningthread, maxSessions, SendFrame, Sendflag, Nagle))
		return;
}

void GameServer::StopServer()
{
	Stop();

	Thread_Destroy();

	// 모듈 객체 종료 함수 호출
	for (int i = 0; i < m_moduleTBLIdx; i++)
	{
		m_moduleTable[i]->Destroy();
	}

	delete m_dbTLS;
	delete m_dbQue;
	delete m_pUserpool;

	CMessage::PoolDestroy();
}

bool GameServer::RegistModule(IModule* pModule)
{
	if (m_moduleTBLIdx >= df_MODULE_MAXCOUNT)
		return false;

	m_moduleTable[m_moduleTBLIdx] = pModule;
	m_moduleTBLIdx++;

	return true;
}

void GameServer::Mem_Init(INT usermax, CHAR* dbip, INT dbport)
{
	std::string schema = "world";

	m_endflag = false;
	m_pUserpool = new CMemoryPool<CUser>;
	m_dbQue = new LFQueue<CMessage*>;
	m_dbTLS = new DBTLS(dbip,dbport, schema);
	m_moduleTable.resize(df_MODULE_MAXCOUNT);
	m_dbEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	InitializeSRWLock(&m_nonuserTableLock);
	InitializeSRWLock(&m_userTableLock);

	Thread_Create();

	m_ctx = new ServerContext(m_userTable, m_nonuserTable, *m_pUserpool, m_userTableLock, m_nonuserTableLock, m_moduleTable);

	// 모듈 객체 초기화 함수 호출
	for (int i = 0; i < m_moduleTBLIdx; i++)
	{
		m_moduleTable[i]->Init(m_ctx);
	}
}

void GameServer::Thread_Create()
{
	m_update = std::thread(&GameServer::UpdateThread, this);
	m_monitor = std::thread(&GameServer::UpdateThread, this);
	m_db = std::thread(&GameServer::DBThread, this);
}

void GameServer::Thread_Destroy()
{
	m_endflag = true;

	if (m_update.joinable())
	{
		m_update.join();
	}

	if (m_monitor.joinable())
	{
		m_monitor.join();
	}

	if (m_db.joinable())
	{
		m_db.join();
	}
}

bool GameServer::OnConnectionRequest(WCHAR* InputIP, unsigned short InputPort)
{
	return false;
}

void GameServer::OnClientJoin(UINT64 SessionID)
{
	AcquireSRWLockExclusive(&m_nonuserTableLock);
	m_nonuserTable.insert(std::pair<UINT64, DWORD>(SessionID, timeGetTime()));
	ReleaseSRWLockExclusive(&m_nonuserTableLock);
}

void GameServer::OnClientLeave(UINT64 SessionID)
{
	std::unordered_map<UINT64, DWORD>::iterator itNon;
	std::unordered_map<UINT64, CUser*>::iterator itOn;

	//Non 유저 자료 구조에서 먼저 찾기
	AcquireSRWLockExclusive(&m_nonuserTableLock);
	itNon = m_nonuserTable.find(SessionID);
	if (itNon != m_nonuserTable.end())
	{
		//찾았으면 제거하고 끝
		m_nonuserTable.erase(itNon);
		ReleaseSRWLockExclusive(&m_nonuserTableLock);
		return;
	}
	ReleaseSRWLockExclusive(&m_nonuserTableLock);

	// 논유저에 없으면 유저에서 찾기
	AcquireSRWLockExclusive(&m_userTableLock);
	itOn = m_userTable.find(SessionID);
	if (itOn == m_userTable.end())
		__debugbreak();

	CUser* pUser = itOn->second;
	m_userTable.erase(itOn);

	for (int i = 0; i < m_moduleTBLIdx; i++)
	{
		m_moduleTable[i]->OnUserDelete(pUser);
	}

	m_pUserpool->Free(pUser);
	ReleaseSRWLockExclusive(&m_userTableLock);

}

void GameServer::OnRecv(UINT64 SessionID, CMessage* pMessage)
{
	WORD type;
	*pMessage >> type;

	if (pMessage->GetLastError())
	{
		Disconnect(SessionID);
		LOG(L"GameServer", en_LOG_LEVEL::dfLOG_LEVEL_ERROR, L"UserRecvMsg::CMessage Flag Error...  / SessionID : %lld ", SessionID);
		return;
	}

	m_moduleTable[type / df_PROTOCOL_RANGE]->OnRecv(SessionID, type, pMessage);

}

void GameServer::MonitorThread()
{
	while (!m_endflag)
	{
		Sleep(1000);

	}
}

void GameServer::DBThread()
{
	CMessage* pMessage = nullptr;

	while (!m_endflag)
	{
		WaitForSingleObject(m_dbEvent, INFINITE);

		while (m_dbQue->Dequeue(pMessage))
		{

		}

	}

	// todo : 큐에 있는것 다 저장 후 스레드 파괴
	while (m_dbQue->Dequeue(pMessage))
	{
		CMessage::Free(pMessage);
	}

}

void GameServer::UpdateThread()
{
	DWORD oldUserTimeoutTick = timeGetTime();
	DWORD oldNonUserTimeoutTick = timeGetTime();
	DWORD curtick;

	while (!m_endflag)
	{
		// 타임 아웃 처리
		curtick = timeGetTime();

		if (curtick - oldUserTimeoutTick >= df_USER_TIMEOUT)
		{
			UserTimeOut();
			oldUserTimeoutTick += df_USER_TIMEOUT;
		}

		if (curtick - oldNonUserTimeoutTick >= df_NONUSER_TIMEOUT)
		{
			NonUserTimeOut();
			oldNonUserTimeoutTick += df_NONUSER_TIMEOUT;
		}

		// 모듈 프레임 로직 처리
		ModuleFrame();
	}
}

void GameServer::ModuleFrame()
{
	DWORD curTick = timeGetTime();

	for (int i = 0; i < m_moduleTBLIdx; i++)
	{
		// 프레임 설정이 따로 안되어 있으면 pass
		if (m_moduleTable[i]->GetFrame() == -1)
			continue;

		// todo : 프레임 밀릴 경우 최대 횟수 정해서 OnUpdate 호출하기
		if (curTick - m_moduleTable[i]->GetOldTime() >= m_moduleTable[i]->GetFrame())
		{
			m_moduleTable[i]->OnUpdate();
			m_moduleTable[i]->SetModuleFrame();
		}

	}
}

void GameServer::UserTimeOut()
{
	DWORD curtick = timeGetTime();
	CUser* pUser = nullptr;
	std::unordered_map<UINT64, CUser*>::iterator it; 

	AcquireSRWLockShared(&m_userTableLock);
	for (it = m_userTable.begin(); it != m_userTable.end(); ++it)
	{
		pUser = it->second;
		
		if (curtick - pUser->GetRecvTime() < df_USER_TIMEOUT)
			continue;

		Disconnect(pUser->GetSessionID());
	}

	ReleaseSRWLockShared(&m_userTableLock);
}

void GameServer::NonUserTimeOut()
{
	DWORD curtick = timeGetTime();
	std::unordered_map<UINT64, DWORD>::iterator it;

	AcquireSRWLockShared(&m_nonuserTableLock);

	for (it = m_nonuserTable.begin(); it != m_nonuserTable.end(); ++it)
	{
		if (curtick - it->second < df_NONUSER_TIMEOUT)
			continue;

		Disconnect(it->first);
	}

	ReleaseSRWLockShared(&m_nonuserTableLock);
}

#include <windows.h>
#include <string>
#include <thread>
#include <unordered_map>
#include <Pdh.h>
#include <codecvt>
#include <mysql.h>
#include "GameDefine.h"
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

#pragma warning (disable:4996)

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
	wstring_convert<std::codecvt_utf8<wchar_t>> converter;

	if (!parser.LoadFile("Config.txt"))
		return;

	Parser::st_Msg bindip;
	parser.GetValue("BIND_IP", &bindip);

	wstring bindstr = converter.from_bytes(bindip.s_ptr);
	
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

	// ��� ��ü ���� �Լ� ȣ��
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
	if (m_moduleTBLIdx >= MODULE_MAX_COUNT)
		return false;

	m_moduleTable[m_moduleTBLIdx] = pModule;
	m_moduleTBLIdx++;

	return true;
}


void GameServer::Mem_Init(INT usermax, CHAR* dbip, INT dbport)
{
	string schema = "world";
	m_endflag = false;
	m_pUserpool = new CMemoryPool<CUser>;
	m_dbQue = new LFQueue<CMessage*>;
	m_dbTLS = new DBTLS(dbip,dbport, schema);
	m_moduleTable.resize(MODULE_MAX_COUNT);
	m_dbEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	InitializeSRWLock(&m_nonuserTableLock);
	InitializeSRWLock(&m_userTableLock);

	Thread_Create();

	m_ctx = new ServerContext(m_userTable, m_nonuserTable, *m_pUserpool, m_userTableLock, m_nonuserTableLock, m_moduleTable, *this, m_moduleTBLIdx);

	// ��� ��ü �ʱ�ȭ �Լ� ȣ��
	for (int i = 0; i < m_moduleTBLIdx; i++)
	{
		m_moduleTable[i]->Init(m_ctx);
	}
}

void GameServer::Thread_Create()
{
	m_update = thread(&GameServer::UpdateThread, this);
	m_monitor = thread(&GameServer::UpdateThread, this);
	m_db = thread(&GameServer::DBThread, this);
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
	unordered_map<UINT64, DWORD>::iterator itNon;
	unordered_map<UINT64, CUser*>::iterator itOn;

	//Non ���� �ڷ� �������� ���� ã��
	AcquireSRWLockExclusive(&m_nonuserTableLock);
	itNon = m_nonuserTable.find(SessionID);
	if (itNon != m_nonuserTable.end())
	{
		//ã������ �����ϰ� ��
		m_nonuserTable.erase(itNon);
		ReleaseSRWLockExclusive(&m_nonuserTableLock);
		return;
	}
	ReleaseSRWLockExclusive(&m_nonuserTableLock);

	// �������� ������ �������� ã��
	AcquireSRWLockExclusive(&m_userTableLock);
	itOn = m_userTable.find(SessionID);
	if (itOn == m_userTable.end())
		__debugbreak();

	CUser* pUser = itOn->second;
	m_userTable.erase(itOn);

	ReleaseSRWLockExclusive(&m_userTableLock);

	for (int i = 0; i < m_moduleTBLIdx; i++)
	{
		m_moduleTable[i]->OnUserDelete(pUser);
	}

	// todo : ĳ���� ������ DB ���� ó��

	m_pUserpool->Free(pUser);
}

void GameServer::OnRecv(UINT64 SessionID, CMessage* pMessage)
{
	m_moduleTable[(WORD) * (pMessage->GetReadPos()) / PROTOCAL_RANGE]->OnRecv(SessionID, pMessage);

	// todo : time�� ���� nonuser, user ���
}

void GameServer::MonitorThread()
{
	time_t start;
	tm* local_time;

	UINT64 loopCnt = 1;
	UINT64 AcptTPSSum = 0;
	UINT64 SendIOSum = 0;
	UINT64 RecvIOSum = 0;

	start = time(NULL);
	local_time = localtime(&start);

	while (!m_endflag)
	{
		Sleep(1000);

		AcptTPSSum += m_AcceptTPS;

		wprintf(L"Start Time : %04d / %02d / %02d, %02d:%02d:%02d\n",
			local_time->tm_year + 1900,
			local_time->tm_mon + 1,
			local_time->tm_mday,
			local_time->tm_hour,
			local_time->tm_min,
			local_time->tm_sec);
		wprintf(L"======================= TPS ����͸� ================================\n");
		wprintf(L"Accept                                        TPS    : (Avg %lld , %d) \n", AcptTPSSum / loopCnt, m_AcceptTPS);
		wprintf(L"SendIOComplete                                TPS    : (Avg %lld, %d) \n", SendIOSum / loopCnt, m_SendIOTPS);
		wprintf(L"RecvIOComplete                                TPS    : (Avg %lld, %d) \n", RecvIOSum / loopCnt, m_RecvIOTPS);


		wprintf(L"====================== ī��Ʈ ����͸� ==============================\n");
		wprintf(L"UserMap / NonUserMap                   Count   : %lld / %lld \n", m_userTable.size(), m_nonuserTable.size());
		wprintf(L"SessionTable                           Count   : %d \n", m_CurSessionCnt);
		wprintf(L"Accept  Total                          Count   : %lld \n", m_AcceptTotal);


		wprintf(L"====================== ��뷮 ����͸� ==============================\n");
		wprintf(L" CMessagePool                 Count : %lld \n", CMessage::m_pMessagePool->GetUseCnt());
		wprintf(L"     UserPool                 Count : %d \n", m_pUserpool->GetUseCnt());



		//wprintf(L"[ CPU Usage : T[%f%] U[%f%] K[%f%]]\n", processtotalsum / loopCnt, processusersum / loopCnt, processkernelsum / loopCnt);
		//wprintf(L"[ Available        Memory Usage : %lf MByte ] [ NonPagedMemory Usage : %lf MByte ]\n", m_pPDH->m_AvailableMemoryVal.doubleValue / (1024 * 1024), m_pPDH->m_NonPagedMemoryVal.doubleValue / (1024 * 1024));
		//wprintf(L"[ Process User     Memory Usage : %lf MByte ]  [ Process NonPaged Memory Usage : %lf KByte ]\n", m_pPDH->m_processUserMemoryVal.doubleValue / (1024 * 1024), m_pPDH->m_processNonPagedMemoryVal.doubleValue / 1024);
		//wprintf(L"[ TCP Retransmitted Avg   Count : %lf /sec  ]  [ TCP Segment Sent  Avg   Count : % lf / sec]\n", tcpretransmitsum / loopCnt, tcpsegmentsentsum / loopCnt);
		loopCnt++;
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

	// todo : ť�� �ִ°� �� ���� �� ������ �ı�
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
		// Ÿ�� �ƿ� ó��
		curtick = timeGetTime();

		//if (curtick - oldUserTimeoutTick >= df_USER_TIMEOUT)
		//{
		//	UserTimeOut();
		//	oldUserTimeoutTick += df_USER_TIMEOUT;
		//}

		//if (curtick - oldNonUserTimeoutTick >= df_NONUSER_TIMEOUT)
		//{
		//	NonUserTimeOut();
		//	oldNonUserTimeoutTick += df_NONUSER_TIMEOUT;
		//}

		// ��� ������ ���� ó��
		ModuleFrame();
	}
}

void GameServer::ModuleFrame()
{
	DWORD curTick = timeGetTime();

	for (int i = 0; i < m_moduleTBLIdx; i++)
	{
		// ������ ������ ���� �ȵǾ� ������ pass
		if (m_moduleTable[i]->GetFrame() == -1)
			continue;

		// todo : ������ �и� ��� �ִ� Ƚ�� ���ؼ� OnUpdate ȣ���ϱ�
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
	unordered_map<UINT64, CUser*>::iterator it; 

	AcquireSRWLockShared(&m_userTableLock);
	for (it = m_userTable.begin(); it != m_userTable.end(); ++it)
	{
		pUser = it->second;
		
		if (curtick - pUser->m_recvTime < USER_TIMEOUT)
			continue;

		Disconnect(pUser->m_sessionID);
	}

	ReleaseSRWLockShared(&m_userTableLock);
}

void GameServer::NonUserTimeOut()
{
	DWORD curtick = timeGetTime();
	unordered_map<UINT64, DWORD>::iterator it;

	AcquireSRWLockShared(&m_nonuserTableLock);

	for (it = m_nonuserTable.begin(); it != m_nonuserTable.end(); ++it)
	{
		if (curtick - it->second < NONUSER_TIMEOUT)
			continue;

		Disconnect(it->first);
	}

	ReleaseSRWLockShared(&m_nonuserTableLock);
}

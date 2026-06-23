#include <windows.h>
#include <thread>
#include <string>
#include <mysql.h>
#include "DBTLS.h"
#include "MemoryPoolTLS.h"
#include "LFQMultiLive.h"
#include "DBJob.h"
#include "TextParser.h"
#include "CDBManager.h"

void CDBManager::Init()
{
	Parser parser;
	if (!parser.LoadFile("DBConfig.txt"))
		__debugbreak();

	Parser::st_Msg DB_IP;
	parser.GetValue("DB_IP", &DB_IP);

	INT  DB_Port;
	parser.GetValue("DB_PORT", &DB_Port);

	std::string schema = "worlddb";
	m_pDBTLS = new DBTLS(DB_IP.s_ptr, DB_Port, schema);
	m_pDBQue = new LFQueueMul<DBJob*>;
	m_DBSaveThread = std::thread(&CDBManager::DBThread, this);
	m_DBEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

void CDBManager::Destroy()
{
	m_pDBQue->Enqueue((DBJob*)1);
	SetEvent(m_DBEvent);

	if (m_DBSaveThread.joinable())
	{
		m_DBSaveThread.join();
	}

	delete m_pDBTLS;
	delete m_pDBQue;
}

void CDBManager::DBThread()
{
	bool endflag = false;
	while (!endflag)
	{
		WaitForSingleObject(m_DBEvent, INFINITE);

		while (m_pDBQue->GetUseSize() > 0)
		{
			DBJob* pJob = nullptr;
			m_pDBQue->Dequeue(pJob);

			// 종료 이벤트면 탈출
			if ((int)pJob == 1)
			{
				endflag = true;
				break;
			}


			// 그게 아니면 Job처리
			pJob->Execute(m_pDBTLS);

			// 만약 replyTo가 nullptr이 아니면 해당 큐에 Job 다시 넣어주기
			if (pJob->replyTo != nullptr)
			{
				pJob->replyTo->Enqueue(pJob);
				continue;
			}

			// replyTo없으면 여기서 객체 지우기
			delete pJob;
		}
	}

	DBJob* pJob = nullptr;
	while(m_pDBQue->GetUseSize() > 0)
	{ 
		m_pDBQue->Dequeue(pJob);

		delete pJob;
	}
}

void CDBManager::EnqueueDBJob(DBJob* pJob)
{
	m_pDBQue->Enqueue(pJob);
	SetEvent(m_DBEvent);
}


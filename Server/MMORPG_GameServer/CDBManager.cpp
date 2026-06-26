#include <windows.h>
#include <thread>
#include <string>
#include <mysql.h>
#include "DBTLS.h"
#include "MemoryPoolTLS.h"
#include "LFQMultiLive.h"
#include "DBJob.h"
#include "TextParser.h"
#include "LatencyHistogram.h"
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

	memset(&m_procTime, 0, sizeof(LatencyHistogram) * (int)DBProcType::Max);
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
		auto start1 = std::chrono::steady_clock::now();
		WaitForSingleObject(m_DBEvent, INFINITE);

		auto end1 = std::chrono::steady_clock::now();

		m_procTime[(int)DBProcType::Block].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end1 - start1).count());

		while (m_pDBQue->GetUseSize() > 0)
		{
			auto start2 = std::chrono::steady_clock::now();

			DBJob* pJob = nullptr;
			m_pDBQue->Dequeue(pJob);

			auto end2 = std::chrono::steady_clock::now();
			m_procTime[(int)DBProcType::Dequeue].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end2 - start2).count());

			// 종료 이벤트면 탈출
			if ((int)pJob == 1)
			{
				endflag = true;
				break;
			}

			auto start3 = std::chrono::steady_clock::now();
			// 그게 아니면 Job처리
			pJob->Execute(m_pDBTLS);
			auto end3 = std::chrono::steady_clock::now();
			m_procTime[(int)DBProcType::Execute].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end3 - start3).count());


			// 만약 replyTo가 nullptr이 아니면 해당 큐에 Job 다시 넣어주기
			if (pJob->replyTo != nullptr)
			{
				auto start4 = std::chrono::steady_clock::now();
				pJob->replyTo->Enqueue(pJob);
				auto end4 = std::chrono::steady_clock::now();
				m_procTime[(int)DBProcType::Enqueue].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end4 - start4).count());
				continue;
			}

			auto start5 = std::chrono::steady_clock::now();
			// replyTo없으면 여기서 객체 지우기
			delete pJob;
			auto end5 = std::chrono::steady_clock::now();
			m_procTime[(int)DBProcType::DeleteJob].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end5 - start5).count());
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


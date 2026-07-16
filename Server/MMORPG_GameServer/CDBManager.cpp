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
	
	std::vector<DBJob*> storejob;
	int index = 0;
	storejob.resize(10);

	bool endflag = false;
	while (!endflag)
	{
		WaitForSingleObject(m_DBEvent, INFINITE);

		bool batchOpen = false;
		int cnt = 0;

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

			// Job이 characterselect거나 uid allocator면 commit해야 하면 commit하기
			if (pJob->flush)
			{
				// 트랜잭션 열려 있으면
				if (batchOpen)
				{
					// 커밋 해주기
					bool success = m_pDBTLS->DB_Post_Query(pJob->result, "COMMIT");
					if (!success)
						__debugbreak();

					batchOpen = false;

					for (int i = 0; i < index; i++)
					{
						delete storejob[i];
						storejob[i] = nullptr;
					}

					index = 0;
				}

				pJob->Execute(m_pDBTLS);

				// 만약 replyTo가 nullptr이 아니면 해당 큐에 Job 다시 넣어주기
				if (pJob->replyTo != nullptr)
				{
					pJob->replyTo->Enqueue(pJob);
					continue;
				}

				// replyTo없으면 여기서 객체 지우기
				delete pJob;

				continue;
			}

			// flush 아닌데 트랜잭션 안열려 있으면 열기
			if (!batchOpen)
			{
				bool success = m_pDBTLS->DB_Post_Query(pJob->result, "START TRANSACTION");
				if (!success)
					__debugbreak();

				batchOpen = true;
				cnt = 0;
				index = 0;
			}


			// 그게 아니면 Job처리
			pJob->Execute(m_pDBTLS);
			cnt++;

			// reply to 없을 때 객체 포인터 저장
			if (pJob->replyTo == nullptr)
			{
				storejob[index++] = pJob;
			}
			else
				pJob->replyTo->Enqueue(pJob);

			// max치 도달하면
			if (cnt >= 10)
			{
				// 커밋 해주기
				bool success = m_pDBTLS->DB_Post_Query(pJob->result, "COMMIT");
				if (!success)
					__debugbreak();

				batchOpen = false;

				for (int i = 0; i < index; i++)
				{
					delete storejob[i];
					storejob[i] = nullptr;
				}

				index = 0;
			}

		}


		// 만약 커밋 안했으면 커밋해주기
		if (batchOpen)
		{
			// 커밋 해주기
			DB_QUERY_RESULT result = DB_QUERY_RESULT::None;
			bool success = m_pDBTLS->DB_Post_Query(result, "COMMIT");
			if (!success)
				__debugbreak();

			batchOpen = false;

			for (int i = 0; i < index; i++)
			{
				delete storejob[i];
				storejob[i] = nullptr;
			}

			index = 0;
		}

	}

	DBJob* pJob = nullptr;
	while (m_pDBQue->GetUseSize() > 0)
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


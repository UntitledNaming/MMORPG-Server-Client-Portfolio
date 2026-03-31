#include <windows.h>
#include <thread>
#include <string>
#include <cpp_redis/cpp_redis>
#include "CMessage.h"
#include "LockFreeMemoryPoolLive.h"
#include "CUser.h"
#include "LogClass.h"
#include "LFQSingleLive.h"
#include "TextParser.h"
#include "IModule.h"
#include "CLanServer.h"
#include "DBTLS.h"
#include "GameServer.h"
#include "ServerContext.h"
#include "Auth.h"

#pragma comment (lib, "cpp_redis.lib")
#pragma comment (lib, "tacopie.lib")

void Auth::Init(ServerContext* ctx)
{
	m_ctx = ctx;
	m_oldTime = timeGetTime();
	m_endflag = false;
	m_pRedisClient = new client;
	m_authEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_authQueue = new LFQueue<JOB*>;
	m_pJobPool = new CMemoryPool<JOB>;
	m_authTPS = 0;

	Parser parser;

	if (!parser.LoadFile("AuthConfig.txt"))
		__debugbreak();

	Parser::st_Msg redisip;
	parser.GetValue("REDIS_IP", &redisip);

	INT redisport;
	parser.GetValue("REDIS_PORT", &redisport);

	// redis 서버 연결
	m_pRedisClient->connect(redisip.s_ptr, redisport);

	// 스레드 생성
	m_auth = thread(&Auth::AuthThread, this);

}

void Auth::Destroy()
{
	CMessage* pMessage = nullptr;

	m_endflag = true;

	if (m_auth.joinable())
	{
		m_auth.join();
	}

	while (m_authQueue->Dequeue(pMessage))
	{
		CMessage::Free(pMessage);
	}

	delete m_authQueue;
	delete m_pRedisClient;
	delete m_pJobPool;
}

void Auth::OnUserCreate(CUser* pUser)
{

}

void Auth::OnUserDelete(CUser* pUser)
{

}

void Auth::OnRecv(UINT64 sessionID, CMessage* pMessage)
{
	JOB* job = m_pJobPool->Alloc();
	job->m_pMessage = pMessage;
	job->m_sessionID = sessionID;

	pMessage->AddRef();

	m_authQueue->Enqueue(job);
	SetEvent(m_authEvent);
}

void Auth::OnUpdate()
{

}

void Auth::AuthThread()
{
	JOB* job = nullptr;

	while (!m_endflag)
	{
		WaitForSingleObject(m_authEvent, INFINITE);

		while (m_authQueue->Dequeue(job))
		{
			AuthProc(job);
			m_pJobPool->Free(job);
		}
	}
}

void Auth::AuthProc(JOB* pJob)
{
	// 인증 처리 및 유저 생성
	CMessage* pMessage = pJob->m_pMessage;
	UINT64 sessionID = pJob->m_sessionID;

	WORD type;
	*pMessage >> type;

	INT64 accountNo;
	*pMessage >> accountNo;

	m_pRedisClient->get(to_string(accountNo), [this, pMessage, sessionID](reply& reply)
		{
			INT64 accountNo = *reinterpret_cast<INT64*>(pMessage->GetReadPos());

			// 토큰 자체가 없으면 해당 유저 연결 끊기
			if (reply.is_null()) {
				LOG(L"GameServer", en_LOG_LEVEL::dfLOG_LEVEL_ERROR, L"AuthProc No Token ... / UniqID : %lld / AccountNo : %lld ", sessionID, accountNo);
				CMessage::Free(pMessage);

				m_ctx->m_gameServer.Disconnect(sessionID);
				return;
			}

			string value = reply.as_string();
			string token(pMessage->GetReadPos() + sizeof(INT64) + ID_MAX * sizeof(WCHAR) + NICK_MAX * sizeof(WCHAR), TOKEN_KEY_MAX);
			CMessage::Free(pMessage);

			if (value == token)
			{
				// 인증 완료 처리
				CUser* pUser = m_ctx->m_pUserpool.Alloc();

				// todo : 유저 초기화(DB 접근 필요)
				pUser->m_sessionID = sessionID;
				pUser->m_xpos = 0;
				pUser->m_ypos = 0;
				pUser->m_hp = 100;
				pUser->m_mp = 100;
				pUser->m_sectorXpos = pUser->m_xpos / FieldConst::SECTOR_SIZE;
				pUser->m_sectorYpos = pUser->m_ypos / FieldConst::SECTOR_SIZE;
				pUser->m_action = CUser::USER_ACTION::STOP;
				pUser->m_inputMask = InputMask::None;
				pUser->m_velocity = 0.f;
				pUser->m_recvTime = timeGetTime();
				memcpy_s(pUser->m_nickName, NICK_MAX, L"my", NICK_MAX);

				// 유저 관리 자료구조에 저장

				AcquireSRWLockExclusive(&m_ctx->m_nonuserTableLock);
				AcquireSRWLockExclusive(&m_ctx->m_userTableLock);
				m_ctx->m_nonuserTable.erase(sessionID);
				m_ctx->m_userTable.insert(pair<UINT64,CUser*>(sessionID,pUser));
				ReleaseSRWLockExclusive(&m_ctx->m_userTableLock);
				ReleaseSRWLockExclusive(&m_ctx->m_nonuserTableLock);

				// 모든 모듈에 유저 생성 전달
				UINT count = m_ctx->m_moduleIdx;
				for (int i = 0; i < count; i++)
				{
					m_ctx->m_moduleTable[i]->OnUserCreate(pUser);
				}

			}

			// 토큰이 다르면 해당 세션 연결 끊기
			else
			{
				LOG(L"GameServer", en_LOG_LEVEL::dfLOG_LEVEL_ERROR, L"AuthProc TokenInvalid ... / UniqID : %lld / AccountNo : %lld / RedisToken : %s / User Token : %s ", sessionID, accountNo, value, token);
				m_ctx->m_gameServer.Disconnect(sessionID);
			}

			// Redis에 토큰 삭제 (비동기 방식)
			vector<string> delvec;
			delvec.push_back(to_string(accountNo));

			m_pRedisClient->del(delvec, [](cpp_redis::reply& del_reply) {

				});

			m_pRedisClient->commit();

			return;
		});

	m_pRedisClient->commit();

}

#pragma once
#include "GameDefine.h"

using namespace AuthConst;
using namespace std;
using namespace cpp_redis;

class Auth : IModule
{
public:
	struct st_JOB
	{
		UINT64     m_sessionID;
		CMessage*  m_pMessage;
	}typedef JOB;

public:
	Auth() = default;
	~Auth() = default;

	virtual void Init(ServerContext* ctx) override;
	virtual void Destroy() override;
	virtual void OnUserCreate(CUser* pUser) override;
	virtual void OnUserDelete(CUser* pUser) override;
	virtual void OnRecv(UINT64 sessionID, CMessage* pMessage) override;
	virtual void OnUpdate() override;

	        void AuthThread();
			void AuthProc(JOB* pJob);
private:
	thread              m_auth;
	client*             m_pRedisClient;
	LFQueue<JOB*>*      m_authQueue;
	CMemoryPool<JOB>*   m_pJobPool;
	HANDLE              m_authEvent;
	BOOL                m_endflag;
	LONG                m_authTPS;
};
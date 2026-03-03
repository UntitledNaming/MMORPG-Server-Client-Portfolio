#pragma once

#define df_MODULE_MAXCOUNT  100
#define df_PROTOCOL_RANGE   1000
#define df_NONUSER_TIMEOUT  4000
#define df_USER_TIMEOUT     40000

class GameServer : CLanServer
{
public:
	GameServer();
	~GameServer();

	void RunServer();
	void StopServer();
	bool RegistModule(IModule* pModule);

private:
	void Mem_Init(INT usermax, CHAR* dbip, INT dbport);
	void Thread_Create();
	void Thread_Destroy();

	///////////////////////////////////
	// 네트워크 라이브러리 콜백 함수 //
	///////////////////////////////////
	bool  OnConnectionRequest(WCHAR* InputIP, unsigned short InputPort);
	void  OnClientJoin(UINT64 SessionID);
	void  OnClientLeave(UINT64 SessionID);
	void  OnRecv(UINT64 SessionID, CMessage* pMessage);

	///////////////////////////////////
    // 게임 서버 클래스 스레드       //
    ///////////////////////////////////
	void MonitorThread();
	void DBThread();
	void UpdateThread();

	///////////////////////////////////
    // 프레임 스레드 로직 처리       //
    ///////////////////////////////////
	void ModuleFrame();
	void UserTimeOut();
	void NonUserTimeOut();

private:
	ServerContext*                     m_ctx;

	///////////////////////////////////
    // 게임 서버 스레드 관련 변수    //
    ///////////////////////////////////
	std::thread                        m_monitor;
	std::thread                        m_db;
	std::thread                        m_update;
	BOOL                               m_endflag;

	///////////////////////////////////
	// 유저 관련 멤버변수            //
	///////////////////////////////////
	std::unordered_map<UINT64, CUser*> m_userTable;
	std::unordered_map<UINT64,DWORD>   m_nonuserTable;
	SRWLOCK                            m_userTableLock;
	SRWLOCK                            m_nonuserTableLock;
	CMemoryPool<CUser>*                m_pUserpool;

	///////////////////////////////////
    // 모듈 관련 멤버변수            //
    ///////////////////////////////////
	std::vector<IModule*>              m_moduleTable;
	UINT                               m_moduleTBLIdx;

	///////////////////////////////////
    // DB 관련 멤버변수              //
    ///////////////////////////////////
	DBTLS*                             m_dbTLS;
	LFQueue<CMessage*>*                m_dbQue;
	HANDLE                             m_dbEvent;

};


#pragma once

class CGameLibrary;
class IUser;
class CMessage;

class CGroup
{
protected:
	//////////////////////////////////////////////
	// 그룹 객체 변수
	//////////////////////////////////////////////
	CGameLibrary*       m_pGameLib;            // 게임 라이브러리 네트워크 및 그룹 등 함수 사용하기 위한 멤버
	SRWLOCK             m_GroupLock;           // 그룹을 상속 받는 컨텐츠 직렬 처리를 위한 Lock
	DWORD               m_GroupID;             // 그룹을 상속 받는 컨텐츠의 실제 타입 구분자(Attach 할때 게임 라이브러리가 부여)
    DWORD               m_GroupFrameTime;      // 프레임 로직 돌릴때 프레임(ms 단위)
	DWORD               m_OldTime;             // 프레임 스레드에서 체크할 시간
	BOOL                m_Shared;              // 그룹 수신 메세지 병렬 처리 제어 플래그


public:
	//////////////////////////////////////////////
	// 그룹 객체 모니터링 변수
	//////////////////////////////////////////////
	LONG                               m_RecvTPS;
	LONG                               m_SendTPS;
	LONG                               m_FrameTPS;

public:
	CGroup() = default;
	virtual ~CGroup() = default;

	inline DWORD  GetOldTime()
	{
		return m_OldTime;
	}

	inline void   SetOldTime()
	{
		m_OldTime += m_GroupFrameTime;
	}

	inline DWORD GetGroupID()
	{
		return m_GroupID;
	}

	inline void   SetGroupID(DWORD groupid)
	{
		m_GroupID = groupid;
	}

    inline DWORD  GetGroupFrame()
	{
		return m_GroupFrameTime;
	}

	inline void SetGroupFrame(DWORD groupFrame)
	{
		m_GroupFrameTime = groupFrame;
	}

	inline void   SetGameLib(CGameLibrary* plb)
	{
		m_pGameLib = plb;
	}
	inline void   ExclusiveGroupLock()
	{
		AcquireSRWLockExclusive(&m_GroupLock);
	}
	inline void   ExclusiveGroupUnlock()
	{
		ReleaseSRWLockExclusive(&m_GroupLock);
	}

	inline void SharedGroupLock()
	{
		AcquireSRWLockShared(&m_GroupLock);
	}

	inline void SharedGroupUnlock()
	{
		ReleaseSRWLockShared(&m_GroupLock);
	}

	inline BOOL GetSharedFlag()
	{
		return m_Shared;
	}

	inline void SetSharedFlag(BOOL flag)
	{
		m_Shared = flag;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	// 컨텐츠가 구현할 Callback 함수
	/////////////////////////////////////////////////////////////////////////////////////////
	virtual void  Init(CGameLibrary* p) = 0;
	virtual void  Destroy() = 0;
	virtual void  OnClientJoin(UINT64 sessionID) = 0;
	virtual void  OnClientLeave(UINT64 sessionID) = 0;
	virtual void  OnRecv(UINT64 sessionID, CMessage* pMessage) = 0;
	virtual void  OnIUserMove(UINT64 sessionID, IUser* pUser) = 0;
	virtual void  OnUpdate() = 0;

	/////////////////////////////////////////////////////////////////////////////////////////
	// 게임 라이브러리 클래스 제공함수 래핑
	/////////////////////////////////////////////////////////////////////////////////////////
	bool    SendPacket(UINT64 SessionID, CMessage* pMessage);
	bool    Disconnect(UINT64 SessionID);
	bool    FindIP(UINT64 SessionID, std::wstring& OutIP);
	bool    GroupMove(std::wstring& ToContents, UINT64 sessionID, IUser* pUser);

	LONG    GetAcceptTPS();
	LONG    GetRecvIOTPS();
	LONG    GetSendIOTPS();
	INT64   GetAcceptTotal();
	SHORT   GetCurSessionCount();
	void    SetAcceptTPS(LONG value);
	void    SetRecvIOTPS(LONG value);
	void    SetSendIOTPS(LONG value);
	CGroup* GetGroupPtr(std::wstring& Contents);
};



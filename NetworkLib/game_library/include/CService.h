#pragma once
#include <windows.h>

class CGameLibrary;
class CMessage;

class CService
{
public:
	CService() = default;
	virtual ~CService() = default;

	virtual void Init(CGameLibrary* p) = 0;
	virtual void Destroy() = 0;
	virtual void OnRecv(UINT64 sessionID, CMessage* pMessage) = 0;
	virtual void OnUpdate() = 0;


	inline void AquireSharedLock()
	{
		AcquireSRWLockShared(&m_lock);
	}

	inline void AquireSharedUnlock()
	{
		ReleaseSRWLockShared(&m_lock);
	}

	inline void AquireExclusiveLock()
	{
		AcquireSRWLockExclusive(&m_lock);
	}

	inline void AquireExclusiveUnlock()
	{
		ReleaseSRWLockExclusive(&m_lock);
	}

	inline BOOL GetSharedFlag()
	{
		return m_shared;
	}

	inline DWORD GetServiceFrame()
	{
		return m_serviceFrameTime;
	}

	inline DWORD GetOldTime()
	{
		return m_oldTime;
	}

	inline void SetOldTime()
	{
		m_oldTime += m_serviceFrameTime;
	}


protected:
	CGameLibrary* m_pGamelib;
	SRWLOCK       m_lock = SRWLOCK_INIT;
	UINT16        m_serviceID;
	DWORD         m_serviceFrameTime;
	DWORD         m_oldTime;
	BOOL          m_shared;
};


#pragma once

#include <windows.h>

class CUser;
class CMessage;
class ServerContext;

class IModule
{
public:
	IModule() = default;
	virtual ~IModule() = default;

	virtual void Init(ServerContext* ctx) = 0;
	virtual void Destroy() = 0;
	virtual void OnUserCreate(CUser* pUser) = 0;
	virtual void OnUserDelete(CUser* pUser) = 0;
	virtual void OnRecv(UINT64 sessionID, CMessage* pMessage) = 0;
	virtual void OnUpdate() = 0;

	inline DWORD GetFrame()
	{
		return m_frame;
	}

	inline void SetModuleFrame()
	{
		m_oldTime += m_frame;
	}

	inline DWORD GetOldTime()
	{
		return m_oldTime;
	}

protected:
	DWORD          m_frame = -1;
	DWORD          m_oldTime = 0;
	ServerContext* m_ctx = nullptr;
}; 


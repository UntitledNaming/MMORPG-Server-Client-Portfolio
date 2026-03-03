#pragma once

class CUser
{
public:
	CUser() = default;
	~CUser();

	void Init();

	inline DWORD GetRecvTime()
	{
		return m_recvTime;
	}

	inline UINT64 GetSessionID()
	{
		return m_sessionID;
	}

private:
	UINT64         m_sessionID;
	std::wstring   m_nickName;
	DWORD          m_recvTime;
};


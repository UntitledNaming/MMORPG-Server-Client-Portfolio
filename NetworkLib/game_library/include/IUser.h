#pragma once

class IUser
{
public:
	IUser() = default;
	virtual ~IUser() = default;  

public:
	UINT64 m_sessionID;             // 게임 라이브러리가 전달하는 세션key
};
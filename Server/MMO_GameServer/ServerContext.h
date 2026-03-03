#pragma once
#include <windows.h>
#include <unordered_map>
#include "LockFreeMemoryPoolLive.h"
#include "CUser.h"

class IModule;

struct ServerContext
{
	std::unordered_map<UINT64, CUser*>& m_userTable;
	std::unordered_map<UINT64, DWORD>&  m_nonuserTable;
	std::vector<IModule*>&              m_moduleTable;
	SRWLOCK&                            m_userTableLock;
	SRWLOCK&                            m_nonuserTableLock;
	CMemoryPool<CUser>&                 m_pUserpool;

	ServerContext(std::unordered_map<UINT64, CUser*>& u,
		std::unordered_map<UINT64, DWORD>& nu,
		CMemoryPool<CUser>& pool,
		SRWLOCK& ulk, 
		SRWLOCK& nulk, 
		std::vector<IModule*>& mt) : m_userTable(u), m_nonuserTable(nu),
		m_pUserpool(pool), m_userTableLock(ulk), m_nonuserTableLock(nulk), m_moduleTable(mt)
	{
	};
};
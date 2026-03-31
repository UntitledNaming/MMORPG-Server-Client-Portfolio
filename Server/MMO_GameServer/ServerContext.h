#pragma once
#include <windows.h>
#include <unordered_map>
#include "LockFreeMemoryPoolLive.h"

using namespace std;

class CUser;
class IModule;
class GameServer;

struct ServerContext
{
	unordered_map<UINT64, CUser*>& m_userTable;
	unordered_map<UINT64, DWORD>&  m_nonuserTable;
	vector<IModule*>&              m_moduleTable;
	UINT&                               m_moduleIdx;
	SRWLOCK&                            m_userTableLock;
	SRWLOCK&                            m_nonuserTableLock;
	CMemoryPool<CUser>&                 m_pUserpool;
	GameServer&                         m_gameServer;

	ServerContext(unordered_map<UINT64, CUser*>& u,
		unordered_map<UINT64, DWORD>& nu,
		CMemoryPool<CUser>& pool,
		SRWLOCK& ulk, 
		SRWLOCK& nulk, 
		vector<IModule*>& mt,
		GameServer& gs,
		UINT& midx) : m_userTable(u), m_nonuserTable(nu),
		m_pUserpool(pool), m_userTableLock(ulk), m_nonuserTableLock(nulk), m_moduleTable(mt), m_gameServer(gs), m_moduleIdx(midx)
	{
	};
};
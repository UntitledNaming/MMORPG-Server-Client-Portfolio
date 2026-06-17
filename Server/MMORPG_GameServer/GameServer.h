#pragma once

class CGameLibrary;
class CUserDirectory;
class CDBManager;
class AuthGroup;
class FieldGroup;
class ProcessMonitor;

class GameServer
{
public:
	GameServer();
	~GameServer();

	void Init();
	void Monitoring();

private:
	CGameLibrary*   m_pGameLib = nullptr;
	CDBManager*     m_pDBManager = nullptr;
	AuthGroup*      m_pAuthGroup = nullptr;
	FieldGroup*     m_pFieldGroup = nullptr;
	ProcessMonitor* m_pPDH = nullptr;
	std::thread     m_monitorThread;
	BOOL            m_endFlag;

};


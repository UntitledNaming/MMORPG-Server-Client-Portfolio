#pragma once

class CGameLibrary;
class CUserDirectory;
class CDBManager;
class AuthGroup;
class FieldGroup;

class GameServer
{
public:
	GameServer();
	~GameServer();

	void Init();
	void Monitoring();

private:
	CGameLibrary*   m_pGameLib;
	CDBManager*     m_pDBManager;
	AuthGroup*      m_pAuthGroup;
	FieldGroup*     m_pFieldGroup;

	std::thread     m_monitorThread;
	BOOL            m_endFlag;

};


#pragma once

class GameServer
{
public:
	GameServer();
	~GameServer();

private:
	CGameLibrary   m_gameLib;
	CUserDirectory m_userDirectory;
	CDBManager     m_dbManager;
	AuthGroup      m_authGroup;
	FieldGroup     m_fieldGroup;

};


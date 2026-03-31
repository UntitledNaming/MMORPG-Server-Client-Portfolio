#pragma once

class DBTLS;

class CDBManager
{
public:
	CDBManager() = default;
	~CDBManager() = default;

	void Init();
	void Destroy();
	void DBThread();
	void DBSaveData(CMessage* pMessage);
	void DBLoadUserData(CMessage* pMessage);

private:
	std::thread         m_dbSaveThread;
	DBTLS*              m_pDBTLS;
	LFQueue<CMessage*>* m_pDBQue;
	BOOL                m_endFlag;
};


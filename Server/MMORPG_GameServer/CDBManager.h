#pragma once

class DBTLS;

template<typename T>
class LFQueue;

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
	DBTLS*              m_pDBTLS = nullptr;
	LFQueue<CMessage*>* m_pDBQue = nullptr;
	bool                m_endFlag = false;
};


#pragma once

class  DBTLS;
struct DBJob;

template<typename T>
class LFQueueMul;

class CDBManager
{
public:
	CDBManager() = default;
	~CDBManager() = default;

	void Init();
	void Destroy();
	void DBThread();
	void EnqueueDBJob(DBJob* pJob);

public:
	std::thread         m_DBSaveThread;
	HANDLE              m_DBEvent;
	DBTLS*              m_pDBTLS = nullptr;
	LFQueueMul<DBJob*>* m_pDBQue = nullptr;
};


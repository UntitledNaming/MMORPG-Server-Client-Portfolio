#include <windows.h>
#include <thread>
#include <string>
#include <mysql.h>
#include "DBTLS.h"
#include "MemoryPoolTLS.h"
#include "LFQSingleLive.h"
#include "CMessage.h"
#include "CDBManager.h"

void CDBManager::Init()
{
	m_pDBTLS = new DBTLS;
	m_pDBQue = new LFQueue<CMessage*>;
	m_endFlag = false;
	m_dbSaveThread = std::thread(&CDBManager::DBThread, this);
}

void CDBManager::Destroy()
{

	if (m_dbSaveThread.joinable())
	{
		m_dbSaveThread.join();
	}

	delete m_pDBTLS;
	delete m_pDBQue;
}

void CDBManager::DBThread()
{
	while (!m_endFlag)
	{
		//
	}
}

void CDBManager::DBSaveData(CMessage* pMessage)
{
	pMessage->AddRef();
}

void CDBManager::DBLoadUserData(CMessage* pMessage)
{

}

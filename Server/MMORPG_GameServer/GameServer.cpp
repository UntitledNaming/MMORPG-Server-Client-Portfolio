#include <windows.h>
#include <string>
#include <thread>
#include <vector>
#include <unordered_map>
#include <Pdh.h>
#include <mysql.h>
#include "GameLibDefine.h"
#include "ContentsDefine.h"
#include "ContentsEnum.h"
#include "ContentsStruct.h"
#include "MemoryPoolTLS.h"
#include "CMessage.h"
#include "CService.h"
#include "CGroup.h"
#include "CDBManager.h"
#include "FieldSector.h"
#include "SectorPos.h"
#include "CMonster.h"
#include "FieldGroup.h"
#include "AuthGroup.h"
#include "CGameLibrary.h"
#include "ItemTable.h"
#include "CUserItemStorage.h"
#include "FieldDropItemPool.h"
#include "ItemUIDAllocator.h"
#include "CPUUsage.h"
#include "ProcessMonitor.h"
#include "LogClass.h"
#include "LFQMultiLive.h"
#include "CSizeClassMemoryPoolTLS.h"
#include "DBTLS.h"
#include "DBJob.h"
#include "GameServer.h"

#pragma comment(lib,"Pdh.lib")

CMPoolTLS<UserItem>* m_itemPool = nullptr;

GameServer::GameServer()
{
	std::wstring auth = L"Auth";
	std::wstring field = L"Field";

	Init();

	// 그룹, 서비스 Attach
	m_pGameLib->AttachGroup((CGroup*)m_pAuthGroup, auth);
	m_pGameLib->AttachGroup((CGroup*)m_pFieldGroup, field);

	// 게임라이브러리 작동
	m_pGameLib->Run();
}

GameServer::~GameServer()
{
	// 객체 파괴자 호출
	m_pDBManager->Destroy();

	m_endFlag = true;
	if (m_monitorThread.joinable())
	{
		m_monitorThread.join();
	}

	// 게임 라이브러리 종료(각 객체에서 직렬화 버퍼 사용하기 때문에 게임 라이브러리 먼저 종료하면 직렬화 버퍼 TLS 풀 파괴되어 버림)
	m_pGameLib->Stop();

	CSizeClassMemoryPoolTLS::PoolDestroy();
	CUserItemStorage::ItemPoolDestroy();
	ItemTable::Destroy();
	FieldDropItemPool::Destroy();
}

void GameServer::Init()
{
	CLogClass::GetInstance()->Init(1);
	CUserItemStorage::ItemPoolInit();
	CSizeClassMemoryPoolTLS::PoolInit();
	ItemTable::Init();
	FieldDropItemPool::Init();
	ItemUIDAllocator::Init(1, 10000);


	m_pGameLib = new CGameLibrary;
	m_pDBManager = new CDBManager;
	m_pAuthGroup = new AuthGroup;
	m_pFieldGroup = new FieldGroup;
	m_pPDH = new ProcessMonitor;
	m_endFlag = false;

	// DB 매니저 초기화
	m_pDBManager->Init();

	// DB 매니저 초기화 후 ItemUID 할당하기 
	ItemUIDAllocate();

	// 그룹에게 DBManager 포인터 전달
	m_pAuthGroup->InitDBManager(m_pDBManager);
	m_pFieldGroup->InitDBManager(m_pDBManager);

	m_monitorThread = std::thread(&GameServer::Monitoring, this);
}

void GameServer::Monitoring()
{
	uint64 loopCnt = 1;
	float processtotalsum = 0;
	float processusersum = 0;
	float processkernelsum = 0;
	double tcpretransmitsum = 0;
	double tcpsegmentsentsum = 0;
	double ethernet1sendsum = 0;
	double ethernet2sendsum = 0;

	double tcpretranslog = 0;

	while (!m_endFlag)
	{
		Sleep(1000);

		m_pPDH->UpdateCounter();
		processtotalsum += m_pPDH->ProcessTotal();
		processusersum += m_pPDH->ProcessUser();
		processkernelsum += m_pPDH->ProcessKernel();
		tcpretransmitsum += m_pPDH->m_TCPReTransmitVal.doubleValue;
		tcpsegmentsentsum += m_pPDH->m_TCPSegmentSentVal.doubleValue;
		ethernet1sendsum += m_pPDH->m_EtherNetSendVal1.doubleValue;
		ethernet2sendsum += m_pPDH->m_EtherNetSendVal2.doubleValue;

		wprintf(L"-----------------------------------------------------------------------------------------\n");
		wprintf(L"                                GameLibrary                                              \n");
		wprintf(L"-----------------------------------------------------------------------------------------\n");
		wprintf(L"AcceptTPS             : %d \n", m_pGameLib->m_AcceptTPS);
		wprintf(L"RecvIOTPS             : %d \n", m_pGameLib->m_RecvIOTPS);
		wprintf(L"SendIOTPS             : %d \n", m_pGameLib->m_SendIOTPS);
		wprintf(L"AcceptTotalT          : %lld \n", m_pGameLib->m_AcceptTotal);
		wprintf(L"Current Session Count : %d \n", m_pGameLib->m_CurSessionCnt);
		wprintf(L"-----------------------------------------------------------------------------------------\n");
		wprintf(L"-----------------------------------------------------------------------------------------\n");
		wprintf(L"                                FieldGroup                                               \n");
		wprintf(L"-----------------------------------------------------------------------------------------\n");
		wprintf(L" User          Count       : %lld \n", m_pFieldGroup->UserCount());
		wprintf(L" NonUser       Count       : %lld \n", m_pAuthGroup->GetNonUserCount());
		wprintf(L" FieldDropItem Count       : %lld \n", FieldDropItemPool::GetDropItemUseCount());
		wprintf(L" Sync Count                : %lld \n", m_pFieldGroup->syncCount);
		wprintf(L" Field Frame               : %lld \n", m_pFieldGroup->fieldframe);
		wprintf(L"-----------------------------------------------------------------------------------------\n");
		wprintf(L"-----------------------------------------------------------------------------------------\n");
		wprintf(L"                                DB                                                       \n");
		wprintf(L"-----------------------------------------------------------------------------------------\n");
		wprintf(L" DBQueue Use Count  : %d \n", m_pDBManager->m_pDBQue->GetUseSize());
		wprintf(L"-----------------------------------------------------------------------------------------\n");
		wprintf(L" CMessage     Pool Usage Count  : %lld \n", CMessage::m_pMessagePool->GetUseCnt());
		wprintf(L" 32SizeBlock  Pool Usage Count  : %lld \n", CSizeClassMemoryPoolTLS::m_blockSize32Pool->GetUseCnt());
		wprintf(L" 64SizeBlock  Pool Usage Count  : %lld \n", CSizeClassMemoryPoolTLS::m_blockSize64Pool->GetUseCnt());
		wprintf(L" 128SizeBlock Pool Usage Count  : %lld \n", CSizeClassMemoryPoolTLS::m_blockSize128Pool->GetUseCnt());
		wprintf(L" 256SizeBlock Pool Usage Count  : %lld \n", CSizeClassMemoryPoolTLS::m_blockSize256Pool->GetUseCnt());
		wprintf(L" 512SizeBlock Pool Usage Count  : %lld \n", CSizeClassMemoryPoolTLS::m_blockSize512Pool->GetUseCnt());
		wprintf(L"-----------------------------------------------------------------------------------------\n");
		wprintf(L"[ CPU Usage : T[%f%] U[%f%] K[%f%]]\n", processtotalsum / loopCnt, processusersum / loopCnt, processkernelsum / loopCnt);
		wprintf(L"[ Available        Memory Usage : %lf MByte ] [ NonPagedMemory Usage : %lf MByte ]\n", m_pPDH->m_AvailableMemoryVal.doubleValue / (1024 * 1024), m_pPDH->m_NonPagedMemoryVal.doubleValue / (1024 * 1024));
		wprintf(L"[ Process User     Memory Usage : %lf MByte ]  [ Process NonPaged Memory Usage : %lf KByte ]\n", m_pPDH->m_processUserMemoryVal.doubleValue / (1024 * 1024), m_pPDH->m_processNonPagedMemoryVal.doubleValue / 1024);
		wprintf(L"[ TCP Retransmitted Avg   Count : %lf /sec  ]  [ TCP Segment Sent  Avg   Count : % lf / sec]\n", tcpretransmitsum / loopCnt, tcpsegmentsentsum / loopCnt);


		tcpretranslog = m_pPDH->m_TCPReTransmitVal.doubleValue;
		if (tcpretranslog >= 2000)
		{
			LOG(L"TCP", en_LOG_LEVEL::dfLOG_LEVEL_SYSTEM, L" TCP Retransmitted : %lf ", tcpretranslog);
		}


		m_pGameLib->m_AcceptTPS = 0;
		m_pGameLib->m_RecvIOTPS = 0;
		m_pGameLib->m_SendIOTPS = 0;
		m_pFieldGroup->fieldframe = 0;
		m_pFieldGroup->attackCount = 0;
		m_pFieldGroup->targetupdatePacketCount = 0;
		FieldGroup::movePacketCount = 0;
		FieldGroup::stopPacketCount = 0;

		loopCnt++;
	}
}

void GameServer::ItemUIDAllocate()
{
	ItemUIDRangeAllocateJob* pJob = new ItemUIDRangeAllocateJob;
	m_pDBManager->EnqueueDBJob(pJob);
}

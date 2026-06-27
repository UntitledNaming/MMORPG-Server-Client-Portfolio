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
#include "LatencyHistogram.h"
#include "MonitoringSnapShot.h"
#include "LockFreeMemoryPoolLive.h"
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

	delete m_pStoreQueue;
	delete m_SnapShotPool;

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

	m_SnapShotPool = new CMemoryPool<MonitorSnapshot>;
	m_pStoreQueue = new LFQueueMul<MonitorSnapshot*>;
	m_storeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

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
	m_storeThread = std::thread(&GameServer::StoreThread, this);
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
		wprintf(L" Auth  Frame               : %d \n", m_pAuthGroup->m_FrameTPS);
		wprintf(L" Field Frame               : %d \n", m_pFieldGroup->m_FrameTPS);
		wprintf(L"-----------------------------------------------------------------------------------------\n");
		wprintf(L"-------------------------------------- TPS ----------------------------------------------\n");
		wprintf(L" Move        Req TPS  : %lld \n", m_pFieldGroup->moveReqCount);
		wprintf(L" LeftAttack  Req TPS  : %lld \n", m_pFieldGroup->leftAttackReqCount);
		wprintf(L" SkillUse    Req TPS  : %lld \n", m_pFieldGroup->skillUseReqCount);
		wprintf(L" PickUp Item Req TPS  : %lld \n", m_pFieldGroup->pickupReqCount);
		wprintf(L" Use Item    Req TPS  : %lld \n", m_pFieldGroup->useitemReqCount);
		wprintf(L" Delete Item Req TPS  : %lld \n", m_pFieldGroup->deleteitemReqCount);
		wprintf(L" Swap Item   Req TPS  : %lld \n", m_pFieldGroup->swapitemReqCount);
		wprintf(L" Respawn     Req TPS  : %lld \n", m_pFieldGroup->rewspawnReqCount);
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

		StoreProc();

		m_pGameLib->m_AcceptTPS = 0;
		m_pGameLib->m_RecvIOTPS = 0;
		m_pGameLib->m_SendIOTPS = 0;
		m_pAuthGroup->m_FrameTPS = 0;
		m_pFieldGroup->m_FrameTPS = 0;

		m_pFieldGroup->moveReqCount= 0;
		m_pFieldGroup->leftAttackReqCount = 0;
		m_pFieldGroup->skillUseReqCount = 0;
		m_pFieldGroup->pickupReqCount = 0;
		m_pFieldGroup->useitemReqCount = 0;
		m_pFieldGroup->deleteitemReqCount = 0;
		m_pFieldGroup->swapitemReqCount = 0;
		m_pFieldGroup->rewspawnReqCount = 0;



		CharacterSelectJob::g_TPS[(int)DBJobCount::CharacterSelect] = 0;
		InsertItemJob::g_TPS[(int)DBJobCount::InsertItem] = 0;
		DeleteItemJob::g_TPS[(int)DBJobCount::DeleteItem] = 0;
		ItemCountUpdateJob::g_TPS[(int)DBJobCount::ItemUpdateCount] = 0;
		ItemSlotUpdateJob::g_TPS[(int)DBJobCount::ItemSlotUpdate] = 0;
		CharacterProgressJob::g_TPS[(int)DBJobCount::CharacterProgress] = 0;
		LogOutJob::g_TPS[(int)DBJobCount::LogOut] = 0;

		loopCnt++;
	}
}

void GameServer::ItemUIDAllocate()
{
	ItemUIDRangeAllocateJob* pJob = new ItemUIDRangeAllocateJob;
	pJob->flush = true;
	m_pDBManager->EnqueueDBJob(pJob);
}

void GameServer::StoreThread()
{
	// 파일 1회 오픈 (w=매 실행마다 새 파일, ccs=UTF-8=엑셀/파이썬 친화). 누적 원하면 "a, ccs=UTF-8"
	_wfopen_s(&m_csvFp, L"C:\\Users\\Public\\LogFolder\\monitor.csv", L"w, ccs=UTF-8");
	if (m_csvFp)
		WriteCsvHeader();

	while (!m_endFlag)
	{
		WaitForSingleObject(m_storeEvent, INFINITE);
			
		while (m_pStoreQueue->GetUseSize() > 0)
		{
			// 파일 저장 하기
			MonitorSnapshot* pSnap = nullptr;
			while (m_pStoreQueue->GetUseSize() > 0)
			{
				m_pStoreQueue->Dequeue(pSnap);     
				WriteSnapshot(pSnap);              // 백분위 계산 + 한 줄 기록
				m_SnapShotPool->Free(pSnap);      
			}
		}
	}

	while (m_pStoreQueue->GetUseSize() > 0)
	{
		MonitorSnapshot* pJob = nullptr;
		m_pStoreQueue->Dequeue(pJob);
		m_SnapShotPool->Free(pJob);
	}

	if (m_csvFp) { fclose(m_csvFp); m_csvFp = nullptr; }
}

void GameServer::StoreProc()
{
	/////////////////////////////////////////////////////////////////////////
    // 저장 스레드로 보낼 측정값들 구조체 할당해서 저장하고 저장 시간도 기록
    /////////////////////////////////////////////////////////////////////////
	MonitorSnapshot* pStore = m_SnapShotPool->Alloc();
	memset(pStore, 0, sizeof(MonitorSnapshot));

	pStore->timestampNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
		std::chrono::steady_clock::now().time_since_epoch()).count();
	pStore->concurrentUsers = (int)m_pFieldGroup->UserCount();   // 필드 동접 (또는 m_CurSessionCnt)

	// ── 히스토그램: 라이브 →스냅샷 (SnapshotAndReset이 복사+리셋 동시) ──
	for (int i = 0; i < (int)FieldRecvType::Max; ++i)
		m_pFieldGroup->m_OnRecvMsgProcTime[i].SnapshotAndReset(pStore->fieldRecv[i]);

	m_pFieldGroup->m_OnRecvLockEnterTime.SnapshotAndReset(pStore->fieldRecvLockEnter);

	for (int i = 0; i < (int)FrameProcType::Max; ++i)
		m_pFieldGroup->m_OnUpdateProcTime[i].SnapshotAndReset(pStore->fieldUpdate[i]);

	m_pFieldGroup->m_OnUpdateLockEnterTime.SnapshotAndReset(pStore->fieldUpdateLockEnter);

	for (int i = 0; i < (int)BroadCastType::Max; ++i)
		m_pFieldGroup->m_BroadCastProcTime[i].SnapshotAndReset(pStore->fieldBroadcast[i]);

	for (int i = 0; i < (int)AuthRecvType::Max; ++i)
		m_pAuthGroup->m_OnRecvProcTime[i].SnapshotAndReset(pStore->authRecv[i]);

	m_pAuthGroup->m_OnRecvLockEnterTime.SnapshotAndReset(pStore->authRecvLockEnter);
	m_pAuthGroup->m_OnUpdateLockEnterTime.SnapshotAndReset(pStore->authUpdateLockEnter);
	m_pAuthGroup->m_UpdateWholeProcTime.SnapshotAndReset(pStore->authUpdateWhole);

	// ── 머신(CPU): 이번 틱 순간값 ──
	pStore->totalCPUUsage = m_pPDH->ProcessTotal();
	pStore->userCPUUsage = m_pPDH->ProcessUser();
	pStore->kernnelCPUUsage = m_pPDH->ProcessKernel();


	// ── DB: Job별 TPS (리셋 196~208 전에 읽어야 함) + 큐 깊이(발산) ──
	pStore->dbJobTPS[(int)DBJobCount::CharacterSelect] = CharacterSelectJob::g_TPS[(int)DBJobCount::CharacterSelect];
	pStore->dbJobTPS[(int)DBJobCount::InsertItem] = InsertItemJob::g_TPS[(int)DBJobCount::InsertItem];
	pStore->dbJobTPS[(int)DBJobCount::DeleteItem] = DeleteItemJob::g_TPS[(int)DBJobCount::DeleteItem];
	pStore->dbJobTPS[(int)DBJobCount::ItemUpdateCount] = ItemCountUpdateJob::g_TPS[(int)DBJobCount::ItemUpdateCount];
	pStore->dbJobTPS[(int)DBJobCount::ItemSlotUpdate] = ItemSlotUpdateJob::g_TPS[(int)DBJobCount::ItemSlotUpdate];
	pStore->dbJobTPS[(int)DBJobCount::CharacterProgress] = CharacterProgressJob::g_TPS[(int)DBJobCount::CharacterProgress];
	pStore->dbJobTPS[(int)DBJobCount::LogOut] = LogOutJob::g_TPS[(int)DBJobCount::LogOut];
	pStore->dbJobQueueCount = m_pDBManager->m_pDBQue->GetUseSize();

	// 히스토그램은 복사만 하면 g_QueryProcTime이 안 비워져 누적된다 → SnapshotAndReset로 복사+리셋(필드 히스토그램과 동일).
	for (int i = 0; i < (int)DBJobCount::Max; ++i)
		DBJob::g_QueryProcTime[i].SnapshotAndReset(pStore->dbQueryProcTime[i]);

	for (int i = 0; i < (int)DBProcType::Max; ++i)
	{
		m_pDBManager->m_procTime[i].SnapshotAndReset(pStore->dbProcTime[i]);
	}


	pStore->storeQueueCount = m_pStoreQueue->GetUseSize();

	// ── 네트워크 ──
	pStore->sendIOTPS = m_pGameLib->m_SendIOTPS;
	pStore->recvIOTPS = m_pGameLib->m_RecvIOTPS;
	pStore->tcpSegmentSend = m_pPDH->m_TCPSegmentSentVal.doubleValue;
	pStore->tcpTransmit = m_pPDH->m_TCPReTransmitVal.doubleValue;

	pStore->syncCount = m_pFieldGroup->syncCount;
	pStore->authFrame = m_pAuthGroup->m_FrameTPS;
	pStore->fieldFrame = m_pFieldGroup->m_FrameTPS;

	m_pStoreQueue->Enqueue(pStore);
	SetEvent(m_storeEvent);
}

namespace {
// 컬럼 이름 테이블 (인덱스 → 사람이 읽는 이름) — 각 enum 순서와 1:1
static const WCHAR* kFieldRecvName[] = {
	L"fRecv_Move", L"fRecv_RTT", L"fRecv_Swing", L"fRecv_Skill", L"fRecv_Pickup",
	L"fRecv_UseItem", L"fRecv_DelItem", L"fRecv_Swap", L"fRecv_Respawn" };
static const WCHAR* kFrameName[] = {
	L"fUpd_Whole", L"fUpd_User", L"fUpd_Monster", L"fUpd_Drop" };
static const WCHAR* kBcastName[] = {
	L"bc_CreateChar", L"bc_DelChar", L"bc_MoveInput", L"bc_Swing",
	L"bc_HitResult", L"bc_Skill", L"bc_CreateMon", L"bc_DelMon", L"bc_MoveMon",
	L"bc_StopMon", L"bc_MonHitPlayer", L"bc_CreateDrop", L"bc_DelDrop", L"bc_Respawn" };
static const WCHAR* kAuthRecvName[] = { L"aRecv_Login", L"aRecv_CharSelect" };
static const WCHAR* kDbJobName[] = {
	L"db_CharSelect", L"db_InsertItem", L"db_DelItem", L"db_ItemCount",
	L"db_ItemSlot", L"db_CharProgress", L"db_LogOut" };

// 테이블이 enum이랑 어긋나면 컴파일 단계에서 잡기 (배열 범위 밖 읽기 방지)
static_assert(_countof(kFieldRecvName) == (int)FieldRecvType::Max, "FieldRecv 이름 테이블 불일치");
static_assert(_countof(kFrameName)     == (int)FrameProcType::Max, "Frame 이름 테이블 불일치");
static_assert(_countof(kBcastName)     == (int)BroadCastType::Max, "Broadcast 이름 테이블 불일치");
static_assert(_countof(kAuthRecvName)  == (int)AuthRecvType::Max,  "AuthRecv 이름 테이블 불일치");
static_assert(_countof(kDbJobName)     == (int)DBJobCount::Max,    "DBJob 이름 테이블 불일치");

static const WCHAR* kDbProcName[] = { L"Block", L"Dequeue", L"Execute", L"Enqueue", L"DeleteJob" };
static_assert(_countof(kDbProcName)    == (int)DBProcType::Max,    "DBProc 이름 테이블 불일치");
}

void GameServer::WriteCsvHeader()
{
	fwprintf(m_csvFp, L"timestampNs,users,");

	for (int i = 0; i < (int)FieldRecvType::Max; ++i) HdrHist(m_csvFp, kFieldRecvName[i]);
	HdrHist(m_csvFp, L"fRecv_Lock");
	for (int i = 0; i < (int)FrameProcType::Max; ++i) HdrHist(m_csvFp, kFrameName[i]);
	HdrHist(m_csvFp, L"fUpd_Lock");
	for (int i = 0; i < (int)BroadCastType::Max; ++i) HdrHist(m_csvFp, kBcastName[i]);
	for (int i = 0; i < (int)AuthRecvType::Max; ++i) HdrHist(m_csvFp, kAuthRecvName[i]);
	HdrHist(m_csvFp, L"aRecv_Lock");
	HdrHist(m_csvFp, L"aUpd_Lock");
	HdrHist(m_csvFp, L"aUpd_Whole");

	fwprintf(m_csvFp, L"cpuTotal,cpuUser,cpuKernel,");
	for (int i = 0; i < (int)DBJobCount::Max; ++i) fwprintf(m_csvFp, L"%s,", kDbJobName[i]);
	for (int i = 0; i < (int)DBJobCount::Max; ++i) { WCHAR nm[64]; swprintf(nm, _countof(nm), L"%s_q", kDbJobName[i]); HdrHistTail(m_csvFp, nm); }
	for (int i = 0; i < (int)DBProcType::Max; ++i) { WCHAR nm[64]; swprintf(nm, _countof(nm), L"dbp_%s", kDbProcName[i]); HdrHistTail(m_csvFp, nm); }
	fwprintf(m_csvFp, L"dbQueue,storeQueue,sendIOTPS,recvIOTPS,tcpSeg,tcpRetx,fieldFrame,authFrame,syncCount\n");
}

void GameServer::WriteSnapshot(MonitorSnapshot* s)
{
	if (!m_csvFp) return;

	fwprintf(m_csvFp, L"%lld,%d,", s->timestampNs, s->concurrentUsers);

	// 필드 수신
	for (int i = 0; i < (int)FieldRecvType::Max; ++i) OutHist(m_csvFp, s->fieldRecv[i]);
	OutHist(m_csvFp, s->fieldRecvLockEnter);

	// 필드 프레임
	for (int i = 0; i < (int)FrameProcType::Max; ++i) OutHist(m_csvFp, s->fieldUpdate[i]);
	OutHist(m_csvFp, s->fieldUpdateLockEnter);

	// 필드 브로드캐스트
	for (int i = 0; i < (int)BroadCastType::Max; ++i) OutHist(m_csvFp, s->fieldBroadcast[i]);
	// Auth
	for (int i = 0; i < (int)AuthRecvType::Max; ++i) OutHist(m_csvFp, s->authRecv[i]);
	OutHist(m_csvFp, s->authRecvLockEnter);
	OutHist(m_csvFp, s->authUpdateLockEnter);
	OutHist(m_csvFp, s->authUpdateWhole);

	// CPU
	fwprintf(m_csvFp, L"%.2f,%.2f,%.2f,", s->totalCPUUsage, s->userCPUUsage, s->kernnelCPUUsage);

	// DB Job TPS + Job별 쿼리 처리시간 히스토그램 + 큐 깊이(DB/Store)
	for (int i = 0; i < (int)DBJobCount::Max; ++i) fwprintf(m_csvFp, L"%lld,", s->dbJobTPS[i]);
	for (int i = 0; i < (int)DBJobCount::Max; ++i) OutHistTail(m_csvFp, s->dbQueryProcTime[i]);
	for (int i = 0; i < (int)DBProcType::Max; ++i) OutHistTail(m_csvFp, s->dbProcTime[i]);
	fwprintf(m_csvFp, L"%lld,%lld,", s->dbJobQueueCount, s->storeQueueCount);

	// 네트워크 (recvIOTPS·fieldFrameTPS는 네가 추가한 필드명에 맞춰)
	fwprintf(m_csvFp, L"%ld,%ld,%.0f,%.0f,", s->sendIOTPS, s->recvIOTPS, s->tcpSegmentSend, s->tcpTransmit);

	// 프레임TPS(1순위) 필드/Auth, syncCount
	fwprintf(m_csvFp, L"%ld,%ld,%llu", s->fieldFrame, s->authFrame, s->syncCount);

	fwprintf(m_csvFp, L"\n");
	fflush(m_csvFp);   // 1초당 1회라 부담 0, 크래시 대비 즉시 flush
}

void GameServer::OutHist(FILE* fp, const LatencyHistogram& h)
{
	fwprintf(fp, L"%llu,%llu,%llu,%lld,",
		h.Percentile(50), h.Percentile(95), h.Percentile(99), h.GetCount());
}

void GameServer::HdrHist(FILE* fp, const WCHAR* name)
{
	fwprintf(fp, L"%s_p50,%s_p95,%s_p99,%s_cnt,", name, name, name, name);
}

// 꼬리까지 노출: p99로는 안 잡히는 극단(p99.9) + 최상위 버킷(max). Percentile(100)=비어있지 않은 최상위 버킷 하한.
void GameServer::OutHistTail(FILE* fp, const LatencyHistogram& h)
{
	fwprintf(fp, L"%llu,%llu,%llu,%llu,%llu,%lld,",
		h.Percentile(50), h.Percentile(95), h.Percentile(99),
		h.Percentile(99.9), h.Percentile(100), h.GetCount());
}

void GameServer::HdrHistTail(FILE* fp, const WCHAR* name)
{
	fwprintf(fp, L"%s_p50,%s_p95,%s_p99,%s_p999,%s_max,%s_cnt,", name, name, name, name, name, name);
}
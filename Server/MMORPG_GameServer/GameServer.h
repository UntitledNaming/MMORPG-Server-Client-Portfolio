#pragma once

template <typename T>
class LFQueueMul;

class CGameLibrary;
class CUserDirectory;
class CDBManager;
class AuthGroup;
class FieldGroup;
class ProcessMonitor;
struct MonitorSnapshot;

class GameServer
{
public:
	GameServer();
	~GameServer();

	void Init();
	void Monitoring();
	void ItemUIDAllocate();
	void StoreThread();

private:
	void StoreProc();
	void WriteCsvHeader();                  // 헤더 행 1회
	void WriteSnapshot(MonitorSnapshot* s);
	void OutHist(FILE* fp, const LatencyHistogram& h);
	void HdrHist(FILE* fp, const WCHAR* name);

private:
	CGameLibrary*   m_pGameLib = nullptr;
	CDBManager*     m_pDBManager = nullptr;
	AuthGroup*      m_pAuthGroup = nullptr;
	FieldGroup*     m_pFieldGroup = nullptr;
	ProcessMonitor* m_pPDH = nullptr;
	BOOL            m_endFlag;
	std::thread     m_monitorThread;

	// 저장 관련 변수들
	FILE* m_csvFp = nullptr;            // 모니터 CSV 파일 핸들 (StoreThread 전용)
	std::thread     m_storeThread;
	HANDLE          m_storeEvent;
	LFQueueMul<MonitorSnapshot*>* m_pStoreQueue = nullptr;
	CMemoryPool<MonitorSnapshot>* m_SnapShotPool = nullptr;
};


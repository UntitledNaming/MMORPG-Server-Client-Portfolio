#include <windows.h>
#include <string>
#include <thread>
#include <vector>
#include <unordered_map>
#include "GameLibDefine.h"
#include "ContentsDefine.h"
#include "ContentsEnum.h"
#include "ContentsStruct.h"
#include "MemoryPoolTLS.h"
#include "CMessage.h"
#include "CService.h"
#include "CGroup.h"
#include "ChatService.h"
#include "CUserDirectory.h"
#include "CDBManager.h"
#include "FieldSector.h"
#include "SectorPos.h"
#include "CMonster.h"
#include "FieldGroup.h"
#include "AuthGroup.h"
#include "CGameLibrary.h"
#include "GameServer.h"

GameServer::GameServer()
{
	std::wstring auth = L"Auth";
	std::wstring field = L"Field";

	Init();

	// 그룹, 서비스 Attach
	m_pGameLib->AttachGroup((CGroup*)m_pAuthGroup, auth);
	m_pGameLib->AttachGroup((CGroup*)m_pFieldGroup, field);
	//m_gameLib.AttachService((CService*)&m_chatService);

	// 그 이외 객체 초기화
	m_pDBManager->Init();
	m_pUserDirectory->Init();

	// 게임라이브러리 작동
	m_pGameLib->Run();
}

GameServer::~GameServer()
{
	// 객체 파괴자 호출
	m_pDBManager->Destroy();
	m_pUserDirectory->Destroy();

	m_endFlag = true;
	if (m_monitorThread.joinable())
	{
		m_monitorThread.join();
	}

	// 게임 라이브러리 종료(각 객체에서 직렬화 버퍼 사용하기 때문에 게임 라이브러리 먼저 종료하면 직렬화 버퍼 TLS 풀 파괴되어 버림)
	m_pGameLib->Stop();
}

void GameServer::Init()
{
	m_pGameLib = new CGameLibrary;
	m_pUserDirectory = new CUserDirectory;
	m_pDBManager = new CDBManager;
	m_pAuthGroup = new AuthGroup;
	m_pFieldGroup = new FieldGroup;

	m_endFlag = false;
	m_monitorThread = std::thread(&GameServer::Monitoring, this);
}

void GameServer::Monitoring()
{
	while (!m_endFlag)
	{
		Sleep(1000);

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
		wprintf(L" User Count  : %lld \n", m_pFieldGroup->UserCount());
		wprintf(L" Sync Count  : %lld \n", m_pFieldGroup->syncCount);
		wprintf(L" Attack Count  : %lld \n", m_pFieldGroup->attackCount);
		wprintf(L" TargetUpdate Count  : %d \n", m_pFieldGroup->targetupdatePacketCount);
		wprintf(L" MovePacket Count  : %lld \n", FieldGroup::movePacketCount);
		wprintf(L" StopPacket Count  : %lld \n", FieldGroup::stopPacketCount);
		wprintf(L" Field Frame : %lld \n", m_pFieldGroup->fieldframe);
		wprintf(L"-----------------------------------------------------------------------------------------\n");
		wprintf(L" CMessage Pool Usage Count  : %lld \n", CMessage::m_pMessagePool->GetUseCnt());
		wprintf(L"-----------------------------------------------------------------------------------------\n");




		m_pGameLib->m_AcceptTPS = 0;
		m_pGameLib->m_RecvIOTPS = 0;
		m_pGameLib->m_SendIOTPS = 0;
		m_pFieldGroup->fieldframe = 0;
		m_pFieldGroup->attackCount = 0;
		m_pFieldGroup->targetupdatePacketCount = 0;
		FieldGroup::movePacketCount = 0;
		FieldGroup::stopPacketCount = 0;
	}
}

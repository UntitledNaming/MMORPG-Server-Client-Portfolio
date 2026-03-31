#include <windows.h>
#include <string>
#include <thread>
#include <vector>
#include <unordered_map>
#include "GameLibDefine.h"

#include "ChatService.h"
#include "FieldGroup.h"
#include "CUserDirectory.h"
#include "CGameLibrary.h"
#include "CDBManager.h"
#include "AuthGroup.h"
#include "GameServer.h"

GameServer::GameServer()
{
	std::wstring auth = L"Auth";
	std::wstring field = L"Field";

	// 그룹, 서비스 Attach
	m_gameLib.AttachGroup((CGroup*)&m_authGroup, auth);
	m_gameLib.AttachGroup((CGroup*)&m_fieldGroup, field);
	m_gameLib.AttachService((CService*)&m_chatService);

	// 그 이외 객체 초기화
	m_dbManager.Init();
	m_userDirectory.Init();

	// 게임라이브러리 작동
	m_gameLib.Run();
}

GameServer::~GameServer()
{
	// 객체 파괴자 호출
	m_dbManager.Destroy();
	m_userDirectory.Destroy();

	// 게임 라이브러리 종료(각 객체에서 직렬화 버퍼 사용하기 때문에 게임 라이브러리 먼저 종료하면 직렬화 버퍼 TLS 풀 파괴되어 버림)
	m_gameLib.Stop();
}

#include <windows.h>
#include <string>
#include <unordered_map>
#include <array>
#include <set>
#include <vector>
#include <thread>
#include <mysql.h>
#include "ContentsEnum.h"
#include "ContentsDefine.h"
#include "ContentsProtocol.h"
#include "MemoryPoolTLS.h"
#include "CMessage.h"
#include "Inventory.h"
#include "Equipment.h"
#include "QuickSlot.h"
#include "CUserItemStorage.h"
#include "IUser.h"
#include "CUser.h"
#include "LFQMultiLive.h"
#include "DBTLS.h"
#include "DBJob.h"
#include "CGroup.h"
#include "CDBManager.h"
#include "AuthGroup.h"

using namespace AuthConst;
using namespace AuthProtocol;

void AuthGroup::InitDBManager(CDBManager* ptr)
{
	m_DBManagerPtr = ptr;
}

void AuthGroup::Init(CGameLibrary* p)
{
	m_pGameLib = p;
	m_GroupFrameTime = NONUSER_TIMEOUT;
	m_OldTime = timeGetTime();
	m_Shared = false;
	m_RecvTPS = 0;
	m_SendTPS = 0;
	m_FrameTPS = 0;
	m_pDBJobQueue = new LFQueueMul<DBJob*>;
	InitializeSRWLock(&m_GroupLock);
}

void AuthGroup::Destroy()
{
	DBJob* pJob = nullptr;
	while (m_pDBJobQueue->GetUseSize() > 0)
	{
		m_pDBJobQueue->Dequeue(pJob);

		delete pJob;
	}

	delete m_pDBJobQueue;
	m_pDBJobQueue = nullptr;
}

void AuthGroup::OnClientJoin(uint64 sessionID)
{
	m_nonuserTable.insert(std::pair<uint64, DWORD>(sessionID, timeGetTime()));
}

void AuthGroup::OnClientLeave(uint64 sessionID)
{
	// NonUserMap에서 찾기
	std::unordered_map<uint64, DWORD>::iterator it = m_nonuserTable.find(sessionID);
	if (it != m_nonuserTable.end())
	{
		// 제거
		m_nonuserTable.erase(it);
		return;
	}

	// NonUserMap에 없으면 UserMap에서 찾기
	std::unordered_map<uint64, CUser*>::iterator it2 = m_userTable.find(sessionID);
	if (it2 == m_userTable.end())
		__debugbreak();

	CUser* pUser = it2->second;
	m_userTable.erase(it2);

	CUser::Free(pUser);
}

void AuthGroup::OnRecv(uint64 sessionID, CMessage* pMessage)
{
	uint16 type;
	*pMessage >> type;

	switch (type)
	{
	case PACKET_CS_GAME_LOGIN_REQ:
		LoginRequestProc(sessionID, pMessage);
		break;

	case PACKET_CS_GAME_CHARACTER_SELECT:
		CharacterSelectProc(sessionID, pMessage);
		break;
	}
}

void AuthGroup::OnIUserMove(uint64 sessionID, IUser* pUser)
{

}

void AuthGroup::OnUpdate()
{
	// NonUserTimeOut

	// UserTimeOut

	// DBManager가 전달한 유저 정보 꺼내서 세팅 후 FieldGroup으로 전송
	while (m_pDBJobQueue->GetUseSize() > 0)
	{
		DBJob* pJob = nullptr;
		m_pDBJobQueue->Dequeue(pJob);

		if (pJob == nullptr)
			__debugbreak();

		// Job에 적힌 세션ID가 현재 그룹에 있는지 체크
		std::unordered_map<uint64, CUser*>::iterator it = m_userTable.find(pJob->sessionID);

		// 없으면 Job 가져오는 동안 연결 끊겼으니 pass
		if (it == m_userTable.end())
		{
			delete pJob;
			continue;
		}

		CUser* pUser = it->second;

		// 현재 유저 그룹에 있으면 Job에 유저 객체 전달
		PostAction ret = pJob->OnComplete(this, pUser);
		if (ret == PostAction::MoveToField)
		{
			std::wstring field = L"Field";

			// GroupMove하는데 세션이 Release 플래그 켜져서 유효하지 않을 수 있음.
			// 어차피 유저 객체 삭제는 Release 작업할때 OnClientLeave로 올것임.
			GroupMove(field, pUser->GetSessionID(), pUser);
		}

		delete pJob;
	}

}

void AuthGroup::LoginRequestProc(uint64 sessionID, CMessage* pMessage)
{
	m_nonuserTable.erase(sessionID);

	CUser* pUser = CUser::Alloc();
	pUser->Init(sessionID);

	// todo : Redis에 토큰 조회

	// todo : 인증 실패면 연결 끊기

	// todo : 성공이 유저 객체에 저장

	// todo : 성공시 DB에서 Redis에서 가져온 AccountID에 해당하는 해당 계정이 소유한 캐릭터 이름, 레벨, 외형정보 등을 가져와 클라에게 Send

	m_userTable.insert(std::pair<uint64, CUser*>(sessionID, pUser));
}

void AuthGroup::CharacterSelectProc(uint64 sessionID, CMessage* pMessage)
{
	uint64 characteruid;
	*pMessage >> characteruid;

	// todo : characterUID에 대한 검증.

	CharacterSelectJob* pJob = new CharacterSelectJob;
	pJob->sessionID = sessionID;
	pJob->characterUID = characteruid;
	pJob->accountID = characteruid;   // AccountID랑 CharacterUID를 통일 시키는 가정(원래는 다름)
	pJob->replyTo = m_pDBJobQueue;

	// DB Manager에게 Job 전달
	m_DBManagerPtr->EnqueueDBJob(pJob);
}

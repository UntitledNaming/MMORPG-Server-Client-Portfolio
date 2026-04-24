#include <windows.h>
#include <vector>
#include <unordered_map>
#include <cmath>
#include "ContentsDefine.h"
#include "ContentsEnum.h"
#include "ContentsProtocol.h"
#include "MemoryPoolTLS.h"
#include "CMessage.h"
#include "IUser.h"
#include "CUser.h"
#include "CGroup.h"
#include "FieldGroup.h"

using namespace FieldConst;
using namespace FieldProtocol;

void FieldGroup::Init(CGameLibrary* p)
{
	m_pGameLib = p;
	m_GroupFrameTime = UPDATE_LOOP_TIME;
	m_OldTime = timeGetTime();
	m_Shared = false;
	m_RecvTPS = 0;
	m_SendTPS = 0;
	m_FrameTPS = 0;
	InitializeSRWLock(&m_GroupLock);

	for (int y = 0; y < SECTOR_Y_MAX; y++)
	{
		for (int x = 0; x < SECTOR_X_MAX; x++)
		{
			m_sectors[y][x].m_userArray.m_userCount = 0;
		}
	}
}

void FieldGroup::Destroy()
{

}

void FieldGroup::OnClientJoin(UINT64 sessionID)
{
	// 호출 될 일 없음
}

void FieldGroup::OnClientLeave(UINT64 sessionID)
{
	std::unordered_map<uint64, CUser*>::iterator it;
	it = m_userLookUpTable.find(sessionID);
	if (it == m_userLookUpTable.end())
		__debugbreak();

	// 본인 캐릭터에 대한 삭제 메세지를 각 섹터에 있는 유저에게 보내기
	CUser* pUser = it->second;
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	mpDeleteCharacter(pUser, pMessage);
	SendPacket_SectorAround(pMessage, pUser);

	CMessage::Free(pMessage);

	// 현재 유저 객체 포인터가 있는 배열 인덱스에 해당 유저 객체 배열의 맨 뒤 원소를 가져와 삽입
	UserArray& userArray = m_sectors[pUser->m_sectorYpos][pUser->m_sectorXpos].m_userArray;
	CUser* pOther = userArray.m_userTable[userArray.m_userCount - 1];
	userArray.m_userTable[pUser->m_arrayIdx] = pOther;
	pOther->m_arrayIdx = pUser->m_arrayIdx;
	userArray.m_userCount--;

	CUser::Free(pUser);
}

void FieldGroup::OnRecv(UINT64 sessionID, CMessage* pMessage)
{
	uint16 type;
	*pMessage >> type;

	switch (type)
	{
	case PACKET_CS_UPDATE_CHARACTER_MOVEMENT_INPUT:
		HandleCharacterMovementUpdate(sessionID, pMessage);
		break;

	}
}

void FieldGroup::OnIUserMove(UINT64 sessionID, IUser* pUser)
{
	// 필드 자료구조에 유저 삽입
	CUser* pOnUser = (CUser*)pUser;

	m_userLookUpTable.insert(std::pair<uint64, CUser*>(sessionID, pOnUser));

	UserArray& userArray = m_sectors[pOnUser->m_sectorYpos][pOnUser->m_sectorXpos].m_userArray;
	userArray.m_userTable[userArray.m_userCount] = pOnUser;
	pOnUser->m_arrayIdx = userArray.m_userCount;
	userArray.m_userCount++;

	// 캐릭터 생성 처리

	// 본인 캐릭터 생성 메세지 만들고 보내기
	CMessage* pCreateMyChrToMeMsg = CMessage::Alloc();
	pCreateMyChrToMeMsg->Clear(1);

	mpCreateMyCharacter(pOnUser, pCreateMyChrToMeMsg);
	SendPacket(pOnUser->m_sessionID, pCreateMyChrToMeMsg);

	CMessage::Free(pCreateMyChrToMeMsg);

	// 본인 캐릭터 주변 섹터 찾기
	SectorAround sectAround;
	SectorFind(sectAround, pOnUser->m_sectorXpos, pOnUser->m_sectorYpos);

	// 섹터 순회하면서 캐릭터 생성 메세지 보내기
	for (int i = 0; i < sectAround.m_count; i++)
	{
		uint16 curSecXpos = sectAround.m_Around[i].m_xpos;
		uint16 curSecYpos = sectAround.m_Around[i].m_ypos;

		// 주변 섹터에 본인 캐릭터 생성 메세지 만들고 보내기
		CMessage* pCreateMyChrToOtherMsg = CMessage::Alloc();
		pCreateMyChrToOtherMsg->Clear(1);

		mpCreateOtherCharacter(pOnUser, pCreateMyChrToOtherMsg);
		SendPacket_SectorOne(pCreateMyChrToOtherMsg, curSecXpos, curSecYpos, pOnUser);

		CMessage::Free(pCreateMyChrToOtherMsg);

		// 해당 섹터의 유저 생성 메세지를 만들어 본인 캐릭터에게 전송
		uint16 curUserCount = m_sectors[curSecYpos][curSecXpos].m_userArray.m_userCount;

		// 섹터에 있는 유저 순회
		for (int j = 0; j < curUserCount; j++)
		{
			CUser* pSecUser = m_sectors[curSecYpos][curSecXpos].m_userArray.m_userTable[j];

			// 섹터 유저가 나면 Pass
			if (pSecUser == pOnUser)
				continue;

			CMessage* pCreateOtherChrToMeMsg = CMessage::Alloc();
			pCreateOtherChrToMeMsg->Clear(1);

			mpCreateOtherCharacter(pSecUser, pCreateOtherChrToMeMsg);
			SendPacket(pOnUser->m_sessionID, pCreateOtherChrToMeMsg);

			CMessage::Free(pCreateOtherChrToMeMsg);
		}
	}
}

void FieldGroup::OnUpdate()
{
	MovementProc();
}

bool FieldGroup::SectorRangeCheck(uint16 xpos, uint16 ypos)
{
	if (xpos < 0 || ypos < 0 || xpos >= SECTOR_X_MAX || ypos >= SECTOR_Y_MAX)
		return false;

	return true;
}

void FieldGroup::SectorFind(SectorAround& pAround, uint16 xpos, uint16 ypos)
{
	int cnt = 0;

	uint16 xarray[9] = { -1,0,1,-1,0,1,-1,0,1 };
	uint16 yarray[9] = { -1,-1,-1,0,0,0,1,1,1 };

	for (int i = 0; i < 9; i++)
	{
		if (!SectorRangeCheck(xpos - xarray[i], ypos - yarray[i]))
			continue;

		pAround.m_Around[cnt].m_xpos = xpos - xarray[i];
		pAround.m_Around[cnt].m_ypos = ypos - yarray[i];
		cnt++;
	}

	pAround.m_count = cnt;
}

void FieldGroup::SendPacket_SectorOne(CMessage* pMessage, uint16 xpos, uint16 ypos, CUser* pUser)
{
	vector<CUser*>& userArray = m_sectors[ypos][xpos].m_userArray.m_userTable;
	uint32 count = m_sectors[ypos][xpos].m_userArray.m_userCount;

	for (int i = 0; i < count; i++)
	{
		CUser* pCurUser = userArray[i];

		// 매개인자로 받은 유저와 같은 유저면 메세지 송신 Pass
		if (pCurUser == pUser)
			continue;

	    SendPacket(pCurUser->m_sessionID, pMessage);
	}

}

void FieldGroup::SendPacket_SectorAround(CMessage* pMessage, CUser* pUser)
{
	uint32 secX = pUser->m_sectorXpos;
	uint32 secY = pUser->m_sectorYpos;

	SectorAround around;
	SectorFind(around, secX, secY);

	for (int i = 0; i < around.m_count; i++)
	{
		SendPacket_SectorOne(pMessage, around.m_Around[i].m_xpos, around.m_Around[i].m_ypos, pUser);
	}
}

void FieldGroup::mpCreateMyCharacter(CUser* pUser, CMessage* pMessage)
{
	*pMessage << PACKET_SC_CREATE_MY_CHARACTER;
	*pMessage << pUser->m_sessionID;
	*pMessage << pUser->m_xpos;
	*pMessage << pUser->m_ypos;
	*pMessage << pUser->m_zpos;
	*pMessage << pUser->m_hp;
	*pMessage << pUser->m_mp;
}

void FieldGroup::mpCreateOtherCharacter(CUser* pUser, CMessage* pMessage)
{
	*pMessage << PACKET_SC_CREATE_OTHER_CHARACTER;
	*pMessage << pUser->m_xpos;
	*pMessage << pUser->m_ypos;
	*pMessage << pUser->m_zpos;
	*pMessage << pUser->m_movementYaw;
	*pMessage << pUser->m_hp;
	*pMessage << (uint8)pUser->m_action;
}

void FieldGroup::mpDeleteCharacter(CUser* pUser, CMessage* pMessage)
{
	*pMessage << PACKET_SC_DELETE_CHARACTER;
	*pMessage << pUser->m_sessionID;
}

void FieldGroup::mpCharacterInputUpdate(CUser* pUser, CMessage* pMessage)
{
	*pMessage << PACKET_SC_UPDATE_CHARACTER_MOVEMENT_INPUT;
	*pMessage << pUser->m_sessionID;
	*pMessage << pUser->m_xpos;
	*pMessage << pUser->m_ypos;
	*pMessage << pUser->m_movementYaw;
	*pMessage << (uint8)pUser->m_action;
}

void FieldGroup::mpSyncMyCharacterPosition(CUser* pUser, CMessage* pMessage)
{
	*pMessage << PACKET_SC_SYNC_MY_CHARACTER_POS;
	*pMessage << pUser->m_xpos;
	*pMessage << pUser->m_ypos;
}

void FieldGroup::mpSyncOtherCharacterPosition(CUser* pUser, CMessage* pMessage)
{
	*pMessage << PACKET_SC_SYNC_OTHER_CHARACTER_POS;
	*pMessage << pUser->m_sessionID;
	*pMessage << pUser->m_xpos;
	*pMessage << pUser->m_ypos;
}

void FieldGroup::HandleCharacterMovementUpdate(uint64 sessionID, CMessage* pMessage)
{
	float xpos;
	float ypos;
	float camerayaw;
	uint8 inputmask;
	uint8 action;

	*pMessage >> xpos;
	*pMessage >> ypos;
	*pMessage >> camerayaw;
	*pMessage >> inputmask;
	*pMessage >> action;
	
	// todo : 추출한 데이터 검증(action, inputmask, yaw 범위 검증)

	CUser* pUser = nullptr;

	std::unordered_map<uint64, CUser*>::iterator it = m_userLookUpTable.find(sessionID);
	
	if (it == m_userLookUpTable.end())
		__debugbreak();

	pUser->m_action = (CUser::USER_ACTION)action;
	pUser->m_cameraYaw = camerayaw;
	pUser->m_inputMask = inputmask;
	pUser->m_cameraYaw = camerayaw;


	// 싱크 틀어졌으면 싱크 패킷 및 input Update 패킷 보내기
	if (std::abs(pUser->m_xpos - xpos) >= SYNC_X_RANGE || std::abs(pUser->m_ypos - ypos) >= SYNC_Y_RANGE)
	{
		CMessage* pSyncMyChrMsg = CMessage::Alloc();
		pSyncMyChrMsg->Clear(1);

		mpSyncMyCharacterPosition(pUser, pSyncMyChrMsg);
		SendPacket(pUser->m_sessionID, pSyncMyChrMsg);

		CMessage::Free(pSyncMyChrMsg);


		CMessage* pSyncOthrChrMsg = CMessage::Alloc();
		pSyncOthrChrMsg->Clear(1);

		mpSyncOtherCharacterPosition(pUser, pSyncOthrChrMsg);

		SendPacket_SectorAround(pSyncMyChrMsg, pUser);

		CMessage::Free(pSyncOthrChrMsg);
	}



	// input Update 패킷 뿌리기
	CMessage* pInputUpdateMsg = CMessage::Alloc();
	pInputUpdateMsg->Clear(1);

	mpCharacterInputUpdate(pUser, pInputUpdateMsg);

	SendPacket_SectorAround(pInputUpdateMsg, pUser);

	CMessage::Free(pInputUpdateMsg);

}

void FieldGroup::MovementProc()
{
	std::unordered_map<uint64, CUser*>::iterator it = m_userLookUpTable.begin();

	for (; it != m_userLookUpTable.end(); ++it)
	{
		CUser* pUser = it->second;

		// 현재 유저 상태가 STOP 이거나 InputMask 설정된게 없으면 이동할 필요 없으니 다음 유저로 이동
		if (pUser->m_action == CUser::USER_ACTION::STOP || pUser->m_inputMask == InputMask::None)
			continue;

		float offset = 0.0f;

		// 상쇄 키 입력 발생시 이동 안하고 다음 유저
		if (!GetInputOffset(pUser->m_inputMask, offset))
			continue;

		float moveyaw = pUser->m_cameraYaw + offset;

		// Degree -> Radian으로 변환
		float rad = DegreeToRadian(moveyaw);
		float dirX = cosf(rad);
		float dirY = sinf(rad);

		// 나중에 달리기도 구현시 action 타입에 따라서 xpos 변경
		if (pUser->m_action == CUser::USER_ACTION::WALK)
		{
			pUser->m_xpos += dirX * pUser->m_walkSpeed;
			pUser->m_ypos += dirY * pUser->m_walkSpeed;
		}
		else if (pUser->m_action == CUser::USER_ACTION::RUN)
		{
			pUser->m_xpos += dirX * pUser->m_runSpeed;
			pUser->m_ypos += dirY * pUser->m_runSpeed;
		}
	}
}

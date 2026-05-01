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

size_t FieldGroup::UserCount()
{
	return m_userLookUpTable.size();
}

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
			m_sectors[y][x].m_userArray.m_userTable.resize(SECTOR_USER_DEFAULT_COUNT);
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
	m_userLookUpTable.erase(it);
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

	case PACKET_CS_RTT_SEND:
		HandleRTTMessage(sessionID, pMessage);
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
	fieldframe++;
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

void FieldGroup::CalSectorTransitionMessageTargets(uint16 oldSecXpos, uint16 oldSecYpos, uint16 newSecXpos, uint16 newSecYpos, SectorAround& outDeleteSector, SectorAround& outCreateSector)
{
	bool curSecOverlapflag[9] = { false };
	SectorAround curSec;
	SectorFind(curSec, oldSecXpos, oldSecYpos);
	

	bool newSecOverlapflag[9] = { false };
	SectorAround newSec;
	SectorFind(newSec, newSecXpos, newSecYpos);


	// 겹치는 좌표를 찾아서 이를 제외한 좌표값을 아웃 파라미터에 담기
	for (int i = 0; i < curSec.m_count; i++)
	{
		for (int j = 0; j < newSec.m_count; j++)
		{
			if (curSecOverlapflag[i] == true || newSecOverlapflag[j] == true)
				continue;

			if (curSec.m_Around[i].m_xpos == newSec.m_Around[j].m_xpos && curSec.m_Around[i].m_ypos == newSec.m_Around[j].m_ypos)
			{
				curSecOverlapflag[i] = true;
				newSecOverlapflag[j] = true;
			}
		}
	}

	// 아웃 파라미터에 담기
	int deletecount = 0;
	int createcount = 0;
	for (int i = 0; i < curSec.m_count; i++)
	{
		if (curSecOverlapflag[i] == true)
			continue;

		outDeleteSector.m_Around[deletecount].m_xpos = curSec.m_Around[i].m_xpos;
		outDeleteSector.m_Around[deletecount].m_ypos = curSec.m_Around[i].m_ypos;
		deletecount++;

	}
	outDeleteSector.m_count = deletecount;


	for (int i = 0; i < newSec.m_count; i++)
	{
		if (newSecOverlapflag[i] == true)
			continue;

		outCreateSector.m_Around[deletecount].m_xpos = newSec.m_Around[i].m_xpos;
		outCreateSector.m_Around[deletecount].m_ypos = newSec.m_Around[i].m_ypos;
		createcount++;

	}
	outCreateSector.m_count = deletecount;
}

void FieldGroup::mpCreateMyCharacter(CUser* pUser, CMessage* pMessage)
{
	*pMessage << PACKET_SC_CREATE_MY_CHARACTER;
	*pMessage << pUser->m_xpos;
	*pMessage << pUser->m_ypos;
	*pMessage << pUser->m_zpos;
	*pMessage << pUser->m_hp;
	*pMessage << pUser->m_maxHP;
	*pMessage << pUser->m_mp;
	*pMessage << pUser->m_maxMP;
}

void FieldGroup::mpCreateOtherCharacter(CUser* pUser, CMessage* pMessage)
{
	*pMessage << PACKET_SC_CREATE_OTHER_CHARACTER;
	*pMessage << pUser->m_sessionID;
	*pMessage << pUser->m_xpos;
	*pMessage << pUser->m_ypos;
	*pMessage << pUser->m_zpos;
	*pMessage << pUser->m_movementYaw;
	*pMessage << pUser->m_hp;
	*pMessage << pUser->m_maxHP;
	*pMessage << (uint8)pUser->m_action;
	*pMessage << (uint8)pUser->m_moveMode;
	*pMessage << pUser->m_moveFlag;
}

void FieldGroup::mpDeleteCharacter(CUser* pUser, CMessage* pMessage)
{
	*pMessage << PACKET_SC_DELETE_CHARACTER;
	*pMessage << pUser->m_sessionID;
}

void FieldGroup::mpCharacterMovementUpdate(CUser* pUser, CMessage* pMessage)
{
	*pMessage << PACKET_SC_UPDATE_CHARACTER_MOVEMENT_INPUT;
	*pMessage << pUser->m_sessionID;
	*pMessage << pUser->m_xpos;
	*pMessage << pUser->m_ypos;
	*pMessage << pUser->m_zpos;
	*pMessage << pUser->m_movementYaw;
	*pMessage << pUser->m_moveFlag;
}

void FieldGroup::mpSyncMyCharacterPosition(CUser* pUser, CMessage* pMessage)
{
	*pMessage << PACKET_SC_SYNC_MY_CHARACTER_POS;
	*pMessage << pUser->m_xpos;
	*pMessage << pUser->m_ypos;
	*pMessage << pUser->m_zpos;
}

void FieldGroup::mpSyncOtherCharacterPosition(CUser* pUser, CMessage* pMessage)
{
	*pMessage << PACKET_SC_SYNC_OTHER_CHARACTER_POS;
	*pMessage << pUser->m_sessionID;
	*pMessage << pUser->m_xpos;
	*pMessage << pUser->m_ypos;
	*pMessage << pUser->m_zpos;
}

void FieldGroup::mpRTTEchoMessage(CMessage* pMessage, double Time)
{
	*pMessage << PACKET_SC_RTT_ECHO;
	*pMessage << Time;
}

void FieldGroup::HandleCharacterMovementUpdate(uint64 sessionID, CMessage* pMessage)
{
	float xpos = 0.0f;
	float ypos = 0.0f;
	float zpos = 0.0f;
	float movementyaw = 0.0f;
	bool  moveflag = false;

	*pMessage >> xpos;
	*pMessage >> ypos;
	*pMessage >> zpos;
	*pMessage >> movementyaw;
	*pMessage >> moveflag;
	
	// todo : 추출한 데이터 검증

	CUser* pUser = nullptr;

	std::unordered_map<uint64, CUser*>::iterator it = m_userLookUpTable.find(sessionID);
	
	if (it == m_userLookUpTable.end())
		__debugbreak();

	pUser = it->second;

	pUser->m_movementYaw = movementyaw;
	pUser->m_moveFlag = moveflag;

	// 싱크 틀어졌으면 싱크 패킷 및 input Update 패킷 보내기
	if (std::abs(pUser->m_xpos - xpos) >= SYNC_X_RANGE || std::abs(pUser->m_ypos - ypos) >= SYNC_Y_RANGE)
	{
		// 싱크 발생시 마지막 체크 시간부터 현재 시간이 특정 시간인 10초를 넘었으면 Sync Count를 0으로 밀어줌.
		if (timeGetTime() - pUser->m_lastSyncCheckTime >= SYNC_COUNT_WINDOW_MS)
		{
			pUser->m_syncCount = 0;
			pUser->m_lastSyncCheckTime = timeGetTime();
		}

		pUser->m_syncCount++;

		// 특정 시간동안 해당 유저의 싱크 패킷 횟수가 임계값을 넘을때 해당 유저 끊기
		if (pUser->m_syncCount >= SYNC_MAX_COUNT)
		{
			// todo : 로그
			Disconnect(sessionID);
			return;
		}


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
	else
	{
		// 싱크 안틀어졌으면 클라의 좌표를 서버가 믿어줌.
		pUser->m_xpos = xpos;
		pUser->m_ypos = ypos;
		pUser->m_zpos = zpos;

		uint16 newSecX = (xpos - MAP_WORLD_OFFSET_X) / SECTOR_SIZE;
		uint16 newSecY = (ypos - MAP_WORLD_OFFSET_Y) / SECTOR_SIZE;

		// 변경된 좌표에 해당하는 섹터가 기존 섹터 좌표와 다르면 섹터 업데이트
		if (pUser->m_sectorXpos != newSecX || pUser->m_sectorYpos != newSecY)
		{
			SectorUpdate(pUser, newSecX, newSecY);
		}
		syncCount++;
	}


	// Movement Update 패킷 뿌리기
	CMessage* pInputUpdateMsg = CMessage::Alloc();
	pInputUpdateMsg->Clear(1);

	mpCharacterMovementUpdate(pUser, pInputUpdateMsg);

	SendPacket_SectorAround(pInputUpdateMsg, pUser);

	CMessage::Free(pInputUpdateMsg);

}

void FieldGroup::HandleRTTMessage(uint64 sessionID, CMessage* pMessage)
{
	double recvtime;

	*pMessage >> recvtime;

	CMessage* pRTTMessage = CMessage::Alloc();
	pRTTMessage->Clear(1);
	mpRTTEchoMessage(pRTTMessage, recvtime);

	SendPacket(sessionID, pRTTMessage);

	CMessage::Free(pRTTMessage);

}

void FieldGroup::MovementProc()
{
	std::unordered_map<uint64, CUser*>::iterator it = m_userLookUpTable.begin();

	for (; it != m_userLookUpTable.end(); ++it)
	{
		CUser* pUser = it->second;

		if (it == m_userLookUpTable.begin())
		{
			xpos = it->second->m_xpos;
			ypos = it->second->m_ypos;
			zpos = it->second->m_zpos;
			secxpos = it->second->m_sectorXpos;
			secypos = it->second->m_sectorYpos;
		}


		// 현재 유저의 moveFlag가 
		if (pUser->m_moveFlag == false )
			continue;

		// Degree -> Radian으로 변환
		float rad = DegreeToRadian(pUser->m_movementYaw);
		float dirX = cosf(rad);
		float dirY = sinf(rad);

		pUser->m_xpos += dirX * pUser->m_moveSpeed;
		pUser->m_ypos += dirY * pUser->m_moveSpeed;

		uint16 newSectorXpos = (pUser->m_xpos - MAP_WORLD_OFFSET_X) / SECTOR_SIZE;
		uint16 newSectorYpos = (pUser->m_ypos - MAP_WORLD_OFFSET_Y) / SECTOR_SIZE;

		if (newSectorXpos != pUser->m_sectorXpos || newSectorYpos != pUser->m_sectorYpos)
		{
			SectorUpdate(pUser, newSectorXpos, newSectorYpos);
		}
	}
}

void FieldGroup::SectorUpdate(CUser* pUser, uint16 nextXpos, uint16 nextYpos)
{
	// 현재 섹터에서 삭제 작업
	UserArray& userArray = m_sectors[pUser->m_sectorYpos][pUser->m_sectorXpos].m_userArray;
	CUser* pOther = userArray.m_userTable[userArray.m_userCount - 1];
	userArray.m_userTable[pUser->m_arrayIdx] = pOther;
	pOther->m_arrayIdx = pUser->m_arrayIdx;
	userArray.m_userCount--;

	// 새로운 섹터에 삽입 작업
	UserArray& newUserArray = m_sectors[nextYpos][nextXpos].m_userArray;
	newUserArray.m_userTable[newUserArray.m_userCount] = pUser;
	pUser->m_arrayIdx = newUserArray.m_userCount;
	newUserArray.m_userCount++;


	// 새롭게 보이는 섹터에 캐릭터 생성 메세지, 안보이는 섹터에 캐릭터 삭제 메세지 보내기
	SectorAround DeleteSector;
	SectorAround CreateSector;
	CalSectorTransitionMessageTargets(pUser->m_sectorXpos, pUser->m_sectorYpos, nextXpos, nextYpos, DeleteSector, CreateSector);

	// 캐릭터 삭제 메세지 보내기
	CMessage* pDeleteMsg = CMessage::Alloc();
	pDeleteMsg->Clear(1);

	mpDeleteCharacter(pUser, pDeleteMsg);

	for (int i = 0; i < DeleteSector.m_count; i++)
	{
		SendPacket_SectorOne(pDeleteMsg, DeleteSector.m_Around[i].m_xpos, DeleteSector.m_Around[i].m_ypos, pUser);
	}
	CMessage::Free(pDeleteMsg);


	// 캐릭터 생성 메세지 보내기
	CMessage* pCreateMsg = CMessage::Alloc();
	pCreateMsg->Clear(1);

	mpCreateOtherCharacter(pUser, pCreateMsg);

	for (int i = 0; i < CreateSector.m_count; i++)
	{
		SendPacket_SectorOne(pCreateMsg, CreateSector.m_Around[i].m_xpos, CreateSector.m_Around[i].m_ypos, pUser);
	}
	CMessage::Free(pCreateMsg);

	pUser->m_sectorXpos = nextXpos;
	pUser->m_sectorYpos = nextYpos;
}

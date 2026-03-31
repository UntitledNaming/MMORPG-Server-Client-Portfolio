#include <windows.h>
#include <unordered_map>
#include <vector>
#include <thread>
#include "CMessage.h"
#include "CUser.h"
#include "IModule.h"
#include "DBTLS.h"
#include "CLanServer.h"
#include "LFQSingleLive.h"
#include "GameServer.h"
#include "ServerContext.h"
#include "Field.h"

void Field::Init(ServerContext* ctx)
{
	m_ctx = ctx;
	for (int y = 0; y < SECTOR_Y_MAX; y++)
	{
		for (int x = 0; x < SECTOR_X_MAX; x++)
		{
			m_sectors[y][x].m_userArray.m_userTable.resize(SECTOR_USER_DEFAULT_COUNT);
		}
	}
}

void Field::Destroy()
{

}

void Field::OnUserCreate(CUser* pOnUser)
{
	// ���� ���Ϳ� ���� ���
	WORD sectorXpos = pOnUser->m_sectorXpos;
	WORD sectorYpos = pOnUser->m_sectorYpos;

	Sector& sector = m_sectors[sectorYpos][sectorXpos];
	UserArray& users = sector.m_userArray;
	
	AcquireSRWLockExclusive(&sector.m_sectorLock);
	users.m_userTable[users.m_count] = pOnUser;
	pOnUser->m_arrayIdx = users.m_count;
	users.m_count++;
	ReleaseSRWLockExclusive(&sector.m_sectorLock);


	// ������ �������� ���� ���� �޼��� ������
	CMessage* pCreateMyChrToMeMsg = CMessage::Alloc();
	pCreateMyChrToMeMsg->Clear(1);

	// ���� ĳ���� ���� �޼��� ����� ������
	mpCreateMyCharacter(pOnUser, pCreateMyChrToMeMsg);
	m_ctx->m_gameServer.SendPacket(pOnUser->m_sessionID, pCreateMyChrToMeMsg);

	// ���� ĳ���� �ֺ� ���� ã��
	SectorAround sectAround;
	SectorFind(sectAround, sectorXpos, sectorYpos);

	// ���� ��ȸ�ϸ鼭 ĳ���� ���� �޼��� ������
	UINT16 count = sectAround.m_count;

	// 9�� ���Ϳ� ���� Lock �ɱ�
	for (int i = 0; i < count; i++)
	{
		AcquireSRWLockShared(&m_sectors[sectAround.m_Around[i].m_ypos][sectAround.m_Around[i].m_xpos].m_sectorLock);
	}

	for (int i = 0; i < count; i++)
	{
		WORD curSecXpos = sectAround.m_Around[i].m_xpos;
		WORD curSecYpos = sectAround.m_Around[i].m_ypos;

		// ���� ĳ���Ϳ� ���� ���� �޼����� �� ���Ϳ� �ִ� �������� ������
		CMessage* pCreateMyChrToOtherMsg = CMessage::Alloc();
		pCreateMyChrToOtherMsg->Clear(1);

		mpCreateOtherCharacter(pOnUser, pCreateMyChrToOtherMsg);
		SendPacket_SectorOne(pCreateMyChrToOtherMsg, curSecXpos, curSecYpos, pOnUser);

		// �ش� ������ ���� ���� �޼����� ����� ���� ĳ���Ϳ��� ����
		UINT16 curUserCount = m_sectors[curSecYpos][curSecXpos].m_userArray.m_count;

		// ���Ϳ� �ִ� ���� ��ȸ
		for (int j = 0; j < curUserCount; j++)
		{
			CUser* pSecUser = m_sectors[curSecYpos][curSecXpos].m_userArray.m_userTable[j];

			// ���� ������ ���� Pass
			if (pSecUser == pOnUser)
				continue;

			CMessage* pCreateOtherChrToMeMsg = CMessage::Alloc();
			pCreateOtherChrToMeMsg->Clear(1);

			mpCreateOtherCharacter(pSecUser, pCreateOtherChrToMeMsg);
			m_ctx->m_gameServer.SendPacket(pOnUser->m_sessionID, pCreateOtherChrToMeMsg);
		}

		// Lock Ǯ��
		ReleaseSRWLockShared(&m_sectors[curSecYpos][curSecXpos].m_sectorLock);
	}

}

void Field::OnUserDelete(CUser* pUser)
{
	WORD secX = pUser->m_sectorXpos;
	WORD secY = pUser->m_sectorYpos;

	// 9�� ���Ϳ� �ִ� �����鿡�� �� ĳ���� ���� �޽��� ������
	SectorAround sectAround;
	SectorFind(sectAround, secX, secY);

	// ���� ��ȸ�ϸ鼭 ĳ���� ���� �޼��� ������
	UINT16 count = sectAround.m_count;

	// 9�� ���Ϳ� ���� Lock �ɱ�
	for (int i = 0; i < count; i++)
	{
		AcquireSRWLockShared(&m_sectors[sectAround.m_Around[i].m_ypos][sectAround.m_Around[i].m_xpos].m_sectorLock);
	}

	for (int i = 0; i < count; i++)
	{
		WORD curSecXpos = sectAround.m_Around[i].m_xpos;
		WORD curSecYpos = sectAround.m_Around[i].m_ypos;

		// ���� ĳ���Ϳ� ���� ���� �޼����� �� ���Ϳ� �ִ� �������� ������
		CMessage* pMessage = CMessage::Alloc();
		pMessage->Clear(1);

		mpDeleteCharacter(pUser, pMessage);
		SendPacket_SectorOne(pMessage, curSecXpos, curSecYpos, pUser);

		ReleaseSRWLockShared(&m_sectors[curSecYpos][curSecXpos].m_sectorLock);
	}

	// �迭���� �����ϱ�
	Sector& sector = m_sectors[secY][secX];
	UserArray& users = sector.m_userArray;

	// �迭 �� �ڿ� �ִ� ���� �����͸� 
	AcquireSRWLockExclusive(&sector.m_sectorLock);
	users.m_userTable[pUser->m_arrayIdx] = users.m_userTable[users.m_count - 1];
	users.m_count--;
	ReleaseSRWLockExclusive(&sector.m_sectorLock);
}

void Field::OnRecv(UINT64 sessionID, CMessage* pMessage)
{
	WORD type;
	*pMessage >> type;

	switch (type)
	{
	case PACKET_CS_INPUT_UPDATE:
		HandleInputMask(sessionID, pMessage);
		break;

	default:
		m_ctx->m_gameServer.Disconnect(sessionID);
		break;
	}
}

void Field::OnUpdate()
{

}

bool Field::SectorRangeCheck(WORD xpos, WORD ypos)
{
	if (xpos < 0 || ypos < 0 || xpos >= SECTOR_X_MAX || ypos >= SECTOR_Y_MAX)
		return false;

	return true;
}

void Field::SectorFind(SectorAround& pAround, WORD xpos, WORD ypos)
{
	INT cnt = 0;

	WORD xarray[9] = { -1,0,1,-1,0,1,-1,0,1 };
	WORD yarray[9] = { -1,-1,-1,0,0,0,1,1,1 };

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

void Field::SendPacket_SectorOne(CMessage* pMessage, WORD xpos, WORD ypos, CUser* pUser)
{
	vector<CUser*>& userArray = m_sectors[ypos][xpos].m_userArray.m_userTable;
	UINT16 count = m_sectors[ypos][xpos].m_userArray.m_count;

	for (int i = 0; i < count; i++)
	{
		CUser* pCurUser = userArray[i];

		// �Ű����ڷ� ���� ������ ���� ������ �޼��� �۽� Pass
		if (pCurUser == pUser)
			continue;

		m_ctx->m_gameServer.SendPacket(pCurUser->m_sessionID, pMessage);

	}
}

void Field::SendPacket_SectorAround(CMessage* pMessage, CUser* pUser)
{

}

void Field::LSendPacket_SectorOne(CMessage* pMessage, WORD xpos, WORD ypos, CUser* pUser)
{
	AcquireSRWLockShared(&m_sectors[ypos][xpos].m_sectorLock);
	vector<CUser*>& userArray = m_sectors[ypos][xpos].m_userArray.m_userTable;
	UINT16 count = m_sectors[ypos][xpos].m_userArray.m_count;

	for (int i = 0; i < count; i++)
	{
		CUser* pCurUser = userArray[i];

		// �Ű����ڷ� ���� ������ ���� ������ �޼��� �۽� Pass
		if (pCurUser == pUser)
			continue;

		m_ctx->m_gameServer.SendPacket(pCurUser->m_sessionID, pMessage);
	}
	ReleaseSRWLockShared(&m_sectors[ypos][xpos].m_sectorLock);
}

void Field::LSendPacket_SectorAround(CMessage* pMessage, CUser* pUser)
{
	WORD secX = pUser->m_sectorXpos;
	WORD secY = pUser->m_sectorYpos;

	SectorAround sectAround;
	SectorFind(sectAround, secX, secY);


	// ���� ��ȸ�ϸ鼭 ĳ���� ���� �޼��� ������
	UINT16 count = sectAround.m_count;

	// 9�� ���Ϳ� ���� Lock �ɱ�
	for (int i = 0; i < count; i++)
	{
		AcquireSRWLockShared(&m_sectors[sectAround.m_Around[i].m_ypos][sectAround.m_Around[i].m_xpos].m_sectorLock);
	}

	for (int i = 0; i < count; i++)
	{
		SendPacket_SectorOne(pMessage, sectAround.m_Around[i].m_xpos, sectAround.m_Around[i].m_ypos, pUser);
		ReleaseSRWLockShared(&m_sectors[sectAround.m_Around[i].m_ypos][sectAround.m_Around[i].m_ypos].m_sectorLock);
	}

}

void Field::mpCreateMyCharacter(CUser* pUser, CMessage* pMessage)
{
	*pMessage << PACKET_SC_CREATE_MY_CHARACTER;
	*pMessage << pUser->m_sessionID;
	*pMessage << pUser->m_xpos;
	*pMessage << pUser->m_ypos;
	*pMessage << pUser->m_hp;
	*pMessage << pUser->m_mp;
}

void Field::mpCreateOtherCharacter(CUser* pUser, CMessage* pMessage)
{
	*pMessage << PACKET_SC_CREATE_OTHER_CHARACTER;
	*pMessage << pUser->m_xpos;
	*pMessage << pUser->m_ypos;
	*pMessage << pUser->m_hp;
	*pMessage << (WORD)pUser->m_action;
	*pMessage << pUser->m_inputMask;
}

void Field::mpDeleteCharacter(CUser* pUser, CMessage* pMessage)
{
	*pMessage << PACKET_SC_DELETE_CHARACTER;
	*pMessage << pUser->m_sessionID;
}

void Field::mpUpdateInputMask(CUser* pUser, CMessage* pMessage)
{
	*pMessage << pUser->m_sessionID;
	*pMessage << pUser->m_inputMask;
	*pMessage << pUser->m_xpos;
	*pMessage << pUser->m_ypos;
}

void Field::HandleInputMask(UINT64 sessionID, CMessage* pMessage)
{
	WORD inputMask;
	WORD xpos;
	WORD ypos;
	CUser* pUser;

	*pMessage >> inputMask;
	*pMessage >> xpos;
	*pMessage >> ypos;

	// ������ inputMask ���� 
	unordered_map<UINT64, CUser*>::iterator it;
	AcquireSRWLockShared(&m_ctx->m_userTableLock);
	it = m_ctx->m_userTable.find(sessionID);
	pUser = it->second;
	ReleaseSRWLockShared(&m_ctx->m_userTableLock);

	pUser->m_inputMask = inputMask;
}


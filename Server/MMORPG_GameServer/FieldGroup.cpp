#include <windows.h>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <chrono>
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
	m_ManaRegenOldTime = timeGetTime();

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

	case PACKET_CS_SWING_LEFT_ATTACK:
		HandleLeftAttackSwing(sessionID, pMessage);
		break;

	case PACKET_CS_STOP_LEFT_ATTACK:
		HandleLeftAttackStop(sessionID, pMessage);
		break;

	case PACKET_CS_USE_SKILL:
		HandleSkillUse(sessionID, pMessage);
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
	ManaRegen();
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

void FieldGroup::SendPacket_SectorAround(CMessage* pMessage, CUser* pUser, bool userSend)
{
	uint32 secX = pUser->m_sectorXpos;
	uint32 secY = pUser->m_sectorYpos;

	SectorAround around;
	SectorFind(around, secX, secY);

	for (int i = 0; i < around.m_count; i++)
	{
		if (userSend)
		{
			SendPacket_SectorOne(pMessage, around.m_Around[i].m_xpos, around.m_Around[i].m_ypos, nullptr);
			continue;
		}

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

		outCreateSector.m_Around[createcount].m_xpos = newSec.m_Around[i].m_xpos;
		outCreateSector.m_Around[createcount].m_ypos = newSec.m_Around[i].m_ypos;
		createcount++;

	}
	outCreateSector.m_count = createcount;
}

bool FieldGroup::IsAlreadyPushed(const SecPos* arr, int count, uint16 sx, uint16 sy)
{
	for (int i = 0; i < count; ++i)
	{
		if (arr[i].m_secXpos == sx && arr[i].m_secYpos == sy)
			return true;
	}
	
	return false;
}

uint64 FieldGroup::GetServerTimeMs()
{
	auto now = std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

void FieldGroup::CollectHitTarget(EServerAbilitySlot skillSlot, float attackYaw, CUser* attacker, std::vector<CUser*>& outHitPlayer, std::vector<CMonster*>& outHitMonster, uint8& outHitPlayerCount, uint8& outHitMonsterCount)
{
	switch (skillSlot)
	{
	case EServerAbilitySlot::LeftAttack:
		CollectHitTaget_LeftAttack(attackYaw, attacker, outHitPlayer, outHitMonster, outHitPlayerCount, outHitMonsterCount);
		break;
	}
}

void FieldGroup::CollectHitTaget_LeftAttack(float attackYaw, CUser* attacker, std::vector<CUser*>& outHitPlayer, std::vector<CMonster*>& outHitMonster, uint8& outHitPlayerCount, uint8& outHitMonsterCount)
{
	// 공격 방향으로 캐릭터 위치에서 직사각형 그려서 공격범위에 들어오는 섹터 좌표 찾기
	int minSX = (int)((attacker->m_xpos - ClientAttack::LEFTATTACK_RANGE  - FieldConst::MAP_WORLD_OFFSET_X) / SECTOR_SIZE);
	int maxSX = (int)((attacker->m_xpos + ClientAttack::LEFTATTACK_RANGE  - FieldConst::MAP_WORLD_OFFSET_X) / SECTOR_SIZE);
	int minSY = (int)((attacker->m_ypos - ClientAttack::LEFTATTACK_RANGE  - FieldConst::MAP_WORLD_OFFSET_Y) / SECTOR_SIZE);
	int maxSY = (int)((attacker->m_ypos + ClientAttack::LEFTATTACK_RANGE  - FieldConst::MAP_WORLD_OFFSET_Y) / SECTOR_SIZE);

	minSX = max(0, minSX);
	minSY = max(0, minSY);
	maxSX = min(SECTOR_X_MAX - 1, maxSX);
	maxSY = min(SECTOR_Y_MAX - 1, maxSY);

	// 해당 섹터들 순회하면서 공격 범위 안에 있는지 체크
	uint8 hitplayerCount = 0;
	uint8 hitmonsterCount = 0;
	for (uint16 sy = minSY; sy <= maxSY; sy++)
	{
		for (uint16 sx = minSX; sx <= maxSX; sx++)
		{
			int count = m_sectors[sy][sx].m_userArray.m_userCount;
			for (int i = 0; i < count; i++)
			{
				CUser* targetPlayer = m_sectors[sy][sx].m_userArray.m_userTable[i];

				if (targetPlayer == attacker)
					continue;

				if (IsInAttackCone(Vec2{attacker->m_xpos, attacker->m_ypos},attackYaw,ClientAttack::LEFTATTACK_RANGE, ClientAttack::LEFATTACK_HALF_ANGLE, 
					Vec2{ targetPlayer->m_xpos,targetPlayer->m_ypos }))
				{
					outHitPlayer[hitplayerCount++] = targetPlayer;
				}

				// todo : 몬스터
			}
		}
	}

	outHitPlayerCount = hitplayerCount;
	outHitMonsterCount = hitmonsterCount;

}

bool FieldGroup::IsInAttackCone(const Vec2& attackPos, float attackYaw, float range, float attackHalfAngle, const Vec2& targetPos)
{
	float dx = targetPos.m_xpos - attackPos.m_xpos;
	float dy = targetPos.m_ypos - attackPos.m_ypos;

	float distSq = dx * dx + dy * dy; // 거리 제곱
	float rangeSq = range * range;    // 사거리 제곱

	// 사거리 밖이면 false
	if (distSq > rangeSq)
		return false;

	// 공격자와 거리가 매우 가까우면 방향상관없이 맞는 처리
	if (distSq <= 0.0001f)
		return false;

	Vec2 forward;
	forward.m_xpos = cosf(DegreeToRadian(attackYaw));
	forward.m_ypos = sinf(DegreeToRadian(attackYaw));

	// 공격 방향 단위 벡터와 내 위치에서 타겟 방향으로의 위치벡터의 내적
	// 다른 말로 내 위치에서 타겟 방향으로의 벡터를 공격 방향 단위 벡터 위로 투영시킨값
	// forwrad . toTarget = |forward| * |toTarget| * cosTheta;
	float dot = dx * forward.m_xpos + dy * forward.m_ypos;

	if (dot <= 0.f)
		return false;

	// forward는 단위 벡터임.
	// dot = |toTarget| * cosTheta;
	// cosTheta = dot / | toTarget|
	// cosTheta는 공격 방향 벡터와 공격자 위치에서 타겟방향으로의 벡터의 사잇각임.
	// 이 각도가 설정한 값 아래여야 함.
	// 사잇각 <= HalfAngle을 만족하려면 cos값을 취하면 cos(사잇각) >= cos(HalfAngle)임
	// cosTheta = dot / sqrt(distSq)
	// 지금 cosTheta >= cosHalfAngle을 만족해야 범위 안임.
	// dot / sqrt(distSq) >= cosHalfAngle 인데 양변 제곱하면
	// dot * dot / distSq >= cosHalfAngle * cosHalfAngle
	// dot * dot >= distSq * cosHalfAngle * cosHalfAngle
	float halfAngleRad = DegreeToRadian(attackHalfAngle);
	float cosHalfAngle = cosf(halfAngleRad);

	return dot * dot >= distSq * cosHalfAngle * cosHalfAngle;
}

uint16 FieldGroup::CalDamage(uint16 atk, uint16 def)
{
	constexpr int DEFENSE_SCALE = 100;

	int damage = atk * DEFENSE_SCALE / (DEFENSE_SCALE * def);
	if (damage < 1)
		return 1;

	return damage;
}

void FieldGroup::mpCreateMyCharacter(CUser* pUser, CMessage* pMessage)
{
	*pMessage << PACKET_SC_CREATE_MY_CHARACTER;
	*pMessage << pUser->m_sessionID;
	*pMessage << pUser->m_xpos;
	*pMessage << pUser->m_ypos;
	*pMessage << pUser->m_zpos;
	*pMessage << pUser->m_hp;
	*pMessage << pUser->m_maxHP;
	*pMessage << pUser->m_mp;
	*pMessage << pUser->m_maxMP;
	*pMessage << pUser->m_mpRegenPerSec;
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
	*pMessage << pUser->m_mp;
	*pMessage << pUser->m_maxMP;
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
	*pMessage << GetServerTimeMs();
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
	*pMessage << GetServerTimeMs();
	*pMessage << pUser->m_xpos;
	*pMessage << pUser->m_ypos;
	*pMessage << pUser->m_zpos;
}

void FieldGroup::mpRTTEchoMessage(CMessage* pMessage)
{
	*pMessage << PACKET_SC_RTT_ECHO;
	*pMessage << GetServerTimeMs();
}

void FieldGroup::mpLeftAttackSwing(CUser* pUser, CMessage* pMessage,float attackyaw)
{
	*pMessage << FieldProtocol::PACKET_SC_SWING_LEFT_ATTACK;
	*pMessage << pUser->m_sessionID;
	*pMessage << attackyaw;
	*pMessage << pUser->m_swingInfo.m_lastSwingIdx;
}

void FieldGroup::mpLeftAttackStop(CUser* pUser, CMessage* pMessage)
{
	*pMessage << FieldProtocol::PACKET_SC_STOP_LEFT_ATTACK;
	*pMessage << pUser->m_sessionID;
}

void FieldGroup::mpTargetHit(uint8 hitPlayerCount, uint8 hitMonsterCount, std::vector<CUser*>& hitPlayerArray, std::vector<CMonster*>& hitMonsterArray, CMessage* pMessage)
{
	*pMessage << PACKET_SC_ATTACK_HIT_RESULT;
	*pMessage << hitPlayerCount;
	*pMessage << hitMonsterCount;

	for (int i = 0; i < hitPlayerCount; i++)
	{
		*pMessage << hitPlayerArray[i]->m_sessionID;
		*pMessage << hitPlayerArray[i]->m_hp;
	}

	for (int i = 0; i < hitMonsterCount; i++)
	{
		// todo : 몬스터 넣기
	}
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
	// 1) Z축 값에 대한 검증 필요. 섹터 하나가 100m이니 그 안에 Zone을 만드는데 Zone의 구역을 직육면체로 정의함.
	//    이 Zone들 마다 속성들이 있음. Z축이 허용되는 지역이 있을 것이고 그게 안되는 지역이 있을 것임.
	//

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

		syncCount++;
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
	mpRTTEchoMessage(pRTTMessage);

	SendPacket(sessionID, pRTTMessage);

	CMessage::Free(pRTTMessage);

}

void FieldGroup::HandleLeftAttackSwing(uint64 sessionID, CMessage* pMessage)
{
	float attackyaw;
	uint8 swingindex;

	*pMessage >> attackyaw;
	*pMessage >> swingindex;

	// 정상 범위 벗어나면 연결 끊기
	if (swingindex <= 0 || swingindex >= 5 )
	{
		Disconnect(sessionID);
		return;
	}

	CUser* pUser = nullptr;

	std::unordered_map<uint64, CUser*>::iterator it = m_userLookUpTable.find(sessionID);
	if (it == m_userLookUpTable.end())
		__debugbreak();

	pUser = it->second;

	bool dis = false;

	// 스윙 패킷이 처음 왔거나 4번째 패킷 보내고 다시 몽타주 재생하여 첫번째 스윙 패킷을 보낸 경우 
	if (pUser->m_swingInfo.m_lastSwingIdx == 0 || pUser->m_swingInfo.m_lastSwingIdx == 4)
	{
		pUser->m_swingInfo.m_lastSwingIdx = 1;
	}

	// 그게 아니면 기존 index 증가
	else
	{
		pUser->m_swingInfo.m_lastSwingIdx++;
	}

	// 유저 swingindex랑 패킷으로 받은 swingindex가 다르면 연결 끊기
	if (pUser->m_swingInfo.m_lastSwingIdx != swingindex)
	{
		Disconnect(sessionID);
		return;
	}

	// 이전 swing 받은 시간과 다음 swing 받은 시간이 600ms 아래면 비정상 유저로 간주
	uint32 curTime = timeGetTime();
	if (curTime - pUser->m_swingInfo.m_lastSwingRecvTime <= ClientAttack::LEFTATTACK_SWING_INTERVAL )
	{
		Disconnect(sessionID);
		return;
	}

	pUser->m_swingInfo.m_lastSwingRecvTime = curTime;

	// 공격자의 위치, 공격 타입을 매개인자로 전달하여 피격자들 찾기
	std::vector<CUser*> targetHitPlayer;
	std::vector<CMonster*> targetHitMonster;
	targetHitPlayer.resize(ClientAttack::LEFTATTACK_MAX_HIT_COUNT);
	targetHitMonster.resize(ClientAttack::LEFTATTACK_MAX_HIT_COUNT);

	uint8 hitplayerCount = 0;
	uint8 hitmonsterCount = 0;
	CollectHitTarget(EServerAbilitySlot::LeftAttack, attackyaw, pUser, targetHitPlayer, targetHitMonster, hitplayerCount, hitmonsterCount);

	// 데미지 계산 및 hp 수정
	for (int i = 0; i < hitplayerCount; i++)
	{
		targetHitPlayer[i]->m_hp -=CalDamage(pUser->m_atk, targetHitPlayer[i]->GetDef());
		if (targetHitPlayer[i]->m_hp <= 0)
			targetHitPlayer[i]->m_hp = 0;

	}

	// 공격자 swing 메세지 뿌리기 
	CMessage* pSwingMsg = CMessage::Alloc();
	pSwingMsg->Clear(1);
	mpLeftAttackSwing(pUser, pSwingMsg, attackyaw);

	SendPacket_SectorAround(pSwingMsg, pUser);
	CMessage::Free(pSwingMsg);


	// 피격자 피격 메세지 뿌리기
	CMessage* pHitMsg = CMessage::Alloc();
	pHitMsg->Clear(1);
	mpTargetHit(hitplayerCount, hitmonsterCount, targetHitPlayer, targetHitMonster, pHitMsg);

	SecPos sendflagArray[5];
	int pushCount = 0;


	for (int i = 0; i < hitplayerCount; i++)
	{
		uint16 secX = targetHitPlayer[i]->m_sectorXpos;
		uint16 secY = targetHitPlayer[i]->m_sectorYpos;

		// 이미 메세지 넣은 섹터 좌표면 pass
		if (IsAlreadyPushed(sendflagArray, pushCount, secX, secY))
			continue;

		SendPacket_SectorAround(pHitMsg, targetHitPlayer[i], true);
		sendflagArray[pushCount++] = SecPos{ secX , secY };
	}

	CMessage::Free(pHitMsg);
}

void FieldGroup::HandleLeftAttackStop(uint64 sessionID, CMessage* pMessage)
{
	float attackyaw;

	*pMessage >> attackyaw;

	CUser* pUser = nullptr;

	std::unordered_map<uint64, CUser*>::iterator it = m_userLookUpTable.find(sessionID);
	if (it == m_userLookUpTable.end())
		__debugbreak();

	pUser = it->second;

	pUser->m_swingInfo.m_lastSwingIdx = 0;

	// 공격자 스윙 정지 메세지 뿌리기
	CMessage* pSwingStop = CMessage::Alloc();
	pSwingStop->Clear(1);
	mpLeftAttackStop(pUser, pSwingStop);

	SendPacket_SectorAround(pSwingStop, pUser);
	CMessage::Free(pSwingStop);
}

void FieldGroup::HandleSkillUse(uint64 sessionID, CMessage* pMessage)
{
	uint8 skillslot;
	*pMessage >> skillslot;

	switch (static_cast<EServerAbilitySlot>(skillslot))
	{
	case EServerAbilitySlot::Skill1:
		HandleSkill1Use(sessionID, pMessage);
		break;

	case EServerAbilitySlot::Skill2:
		HandleSkill2Use(sessionID, pMessage);
		break;

	case EServerAbilitySlot::Skill3:
		HandleSkill3Use(sessionID, pMessage);
		break;

	case EServerAbilitySlot::Skill4:
		HandleSkill4Use(sessionID, pMessage);
		break;
	}
}

void FieldGroup::HandleSkill1Use(uint64 sessionID, CMessage* pMessage)
{
	uint32 curTime = timeGetTime();

	CUser* pUser = nullptr;
	std::unordered_map<uint64, CUser*>::iterator it = m_userLookUpTable.find(sessionID);
	if (it == m_userLookUpTable.end())
		__debugbreak();

	pUser = it->second;

	bool Success = false;

	// 쿨 타임이 안 지났거나 mp가 부족하면 스킬 실패
	if (curTime - pUser->m_skillInfo[0].m_skillLastRecvTime < ClientAttack::DEFENCE_BUFF_COOLTIME_SEC
		|| pUser->m_mp < ClientAttack::DEFENCE_BUFF_REQUIRED_MANA)
		Success = false;
	else
		Success = true;

	// todo : 쿨타임 안 지났는데 패킷 지속적으로 보내는게 체크되면 연결 끊기

	CMessage* pUseSkillRes = CMessage::Alloc();
	pUseSkillRes->Clear(1);
	*pUseSkillRes << PACKET_SC_USE_SKILL_RES;
	*pUseSkillRes << static_cast<uint8>(EServerAbilitySlot::Skill1);
	*pUseSkillRes << (uint8)(Success);

	SendPacket(sessionID, pUseSkillRes);
	CMessage::Free(pUseSkillRes);

	if (Success)
	{
		// 성공했으면 방어력 증가시키고 MP 줄이기
		pUser->SkillInfoUpdate(0, timeGetTime(), true);

		// Skill 사용 패킷 뿌리기
		CMessage* pUseSkillBroad = CMessage::Alloc();
		pUseSkillBroad->Clear(1);
		*pUseSkillBroad << PACKET_SC_USE_SKILL_BROADCAST;
		*pUseSkillBroad << sessionID;
		*pUseSkillBroad << static_cast<uint8>(EServerAbilitySlot::Skill1);

		SendPacket_SectorAround(pUseSkillBroad, pUser);

		CMessage::Free(pUseSkillBroad);
	}

}

void FieldGroup::HandleSkill2Use(uint64 sessionID, CMessage* pMessage)
{
}

void FieldGroup::HandleSkill3Use(uint64 sessionID, CMessage* pMessage)
{
}

void FieldGroup::HandleSkill4Use(uint64 sessionID, CMessage* pMessage)
{

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
			hp = it->second->m_hp;
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

void FieldGroup::ManaRegen()
{
	if (timeGetTime() - m_ManaRegenOldTime < 1000)
		return;

	CUser* targetUser = nullptr;
	std::unordered_map<uint64, CUser*>::iterator it;

	for (it = m_userLookUpTable.begin(); it != m_userLookUpTable.end(); ++it)
	{
		targetUser = it->second;
		targetUser->m_mp += targetUser->m_mpRegenPerSec;
		if (targetUser->m_mp > targetUser->m_maxMP)
			targetUser->m_mp = targetUser->m_maxMP;
	}
}

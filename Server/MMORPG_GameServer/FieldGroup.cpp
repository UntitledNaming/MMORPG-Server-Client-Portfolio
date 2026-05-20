#include <windows.h>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <chrono>
#include "CMonster.h"
#include "ContentsDefine.h"
#include "ContentsEnum.h"
#include "ContentsStruct.h"
#include "ContentsProtocol.h"
#include "MemoryPoolTLS.h"
#include "CMessage.h"
#include "CGroup.h"
#include "PacketBuilder.h"
#include "AttackCollision.h"
#include "FieldSector.h"
#include "SectorPos.h"
#include "IUser.h"
#include "CUser.h"
#include "HitSearchBuilder.h"
#include "FieldGroup.h"


using namespace FieldConst;
using namespace FieldProtocol;

size_t FieldGroup::UserCount()
{
	return m_userLookUpTable.size();
}

void FieldGroup::Init(CGameLibrary* p)
{
	srand(time(NULL));

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
			m_sectors[y][x].Init();
		}
	}

	// 섹터에 몬스터 배치하기
	MonsterSpawnInit();
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

	CMessage* pMessage = PacketBuilder::DeleteCharacter(pUser);
	SendPacket_SectorAround(pMessage, pUser);
	CMessage::Free(pMessage);

	FieldSector& sec = m_sectors[pUser->GetSectorYpos()][pUser->GetSectorXpos()];
	sec.RemoveUser(pUser);

	pUser->Destroy();
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

	FieldSector& sec = m_sectors[pOnUser->GetSectorYpos()][pOnUser->GetSectorXpos()];
	sec.AddUser(pOnUser);

	// 캐릭터 생성 처리

	// 본인 캐릭터 생성 메세지 만들고 보내기
	CMessage* pCreateMyChrToMeMsg = PacketBuilder::CreateMyCharacter(pOnUser);
	SendPacket(pOnUser->GetSessionID(), pCreateMyChrToMeMsg);
	CMessage::Free(pCreateMyChrToMeMsg);

	// 본인 캐릭터 주변 섹터 찾기
	SectorAround sectAround;
	pOnUser->SectorFind(sectAround);

	// 섹터 순회하면서 캐릭터 생성 메세지 보내기
	for (int i = 0; i < sectAround.m_count; i++)
	{
		uint16 curSecXpos = sectAround.m_Around[i].GetX();
		uint16 curSecYpos = sectAround.m_Around[i].GetY();

		// 주변 섹터에 본인 캐릭터 생성 메세지 만들고 보내기
		CMessage* pCreateMyChrToOtherMsg = PacketBuilder::CreateOtherCharacter(pOnUser);
		SendPacket_SectorOne(pCreateMyChrToOtherMsg, curSecXpos, curSecYpos, pOnUser);
		CMessage::Free(pCreateMyChrToOtherMsg);

		// 해당 섹터의 유저 생성 메세지를 만들어 본인 캐릭터에게 전송
		uint16 curUserCount = m_sectors[curSecYpos][curSecXpos].GetUserCount();

		// 섹터에 있는 유저 순회
		for (int j = 0; j < curUserCount; j++)
		{
			CUser* pSecUser = m_sectors[curSecYpos][curSecXpos].GetUser(j);

			// 섹터 유저가 나면 Pass
			if (pSecUser == pOnUser)
				continue;

			CMessage* pCreateOtherChrToMeMsg = PacketBuilder::CreateOtherCharacter(pSecUser);
			SendPacket(pOnUser->GetSessionID(), pCreateOtherChrToMeMsg);
			CMessage::Free(pCreateOtherChrToMeMsg);
		}

		// 섹터 주변 몬스터에 대한 생성 메세지 전송
		uint16 curMonsterCount = m_sectors[curSecYpos][curSecXpos].GetMonsterCount();
		for (int monstercount = 0; monstercount < curMonsterCount; monstercount++)
		{
			CMessage* pCreateMonsterMsg = PacketBuilder::CreateMonster(m_sectors[curSecYpos][curSecXpos].GetMonster(monstercount));
			SendPacket(pOnUser->GetSessionID(), pCreateMonsterMsg);
			CMessage::Free(pCreateMonsterMsg);
		}
	}
}

void FieldGroup::OnUpdate()
{
	MovementProc();
	UserManaRegen();
	fieldframe++;
}

void FieldGroup::SendPacket_SectorOne(CMessage* pMessage, uint16 xpos, uint16 ypos, CUser* pUser)
{
	uint16 count = m_sectors[ypos][xpos].GetUserCount();

	for (int i = 0; i < count; i++)
	{
		CUser* pCurUser = m_sectors[ypos][xpos].GetUser(i);

		// 매개인자로 받은 유저와 같은 유저면 메세지 송신 Pass
		if (pCurUser == pUser)
			continue;

	    SendPacket(pCurUser->GetSessionID(), pMessage);
	}

}

void FieldGroup::SendPacket_SectorAround(CMessage* pMessage, CUser* pUser, bool userSend)
{
	uint32 secX = pUser->GetSectorXpos();
	uint32 secY = pUser->GetSectorYpos();

	SectorAround around;
	pUser->SectorFind(around);

	for (int i = 0; i < around.m_count; i++)
	{
		if (userSend)
		{
			SendPacket_SectorOne(pMessage, around.m_Around[i].GetX(), around.m_Around[i].GetY(), nullptr);
			continue;
		}

		SendPacket_SectorOne(pMessage, around.m_Around[i].GetX(), around.m_Around[i].GetY(), pUser);
	}
}

void FieldGroup::CollectHitTarget(CUser* attacker, HitSearchInfo& hitInfo, HitResult& hitResult)
{
	// 공격 방향으로 캐릭터 위치에서 직사각형 그려서 공격범위에 들어오는 섹터 좌표 찾기
	int minSX = (int)((hitInfo.x - hitInfo.range - FieldConst::MAP_WORLD_OFFSET_X) / SECTOR_SIZE);
	int maxSX = (int)((hitInfo.x + hitInfo.range - FieldConst::MAP_WORLD_OFFSET_X) / SECTOR_SIZE);
	int minSY = (int)((hitInfo.y - hitInfo.range - FieldConst::MAP_WORLD_OFFSET_Y) / SECTOR_SIZE);
	int maxSY = (int)((hitInfo.y + hitInfo.range - FieldConst::MAP_WORLD_OFFSET_Y) / SECTOR_SIZE);

	minSX = max(0, minSX);
	minSY = max(0, minSY);
	maxSX = min(SECTOR_X_MAX - 1, maxSX);
	maxSY = min(SECTOR_Y_MAX - 1, maxSY);

	uint8 hitplayerCount = 0;
	uint8 hitmonsterCount = 0;

	bool hitstopplayer = false;
	bool hitstopmonster = false;

	for (uint16 sy = minSY; sy <= maxSY; sy++)
	{
		for (uint16 sx = minSX; sx <= maxSX; sx++)
		{
			if (hitInfo.bHitUser && !hitstopplayer)
			{
				int count = m_sectors[sy][sx].GetUserCount();
				for (int i = 0; i < count; i++)
				{
					CUser* targetPlayer = m_sectors[sy][sx].GetUser(i);

					if (targetPlayer == attacker || targetPlayer->GetHP() <= 0 || targetPlayer->GetDisconnectFlag())
						continue;

					switch (hitInfo.shape)
					{
					case EHitShape::Cone:
						if (AttackCollision::IsInCone(attacker->GetLocation(), targetPlayer->GetLocation(), hitInfo.range, hitInfo.attackYaw, hitInfo.halfAngleDegree))
						{
							hitResult.HitUserArray[hitplayerCount++] = targetPlayer;
						}
						break;

					case EHitShape::Circle:
						if (AttackCollision::IsInCircle(attacker->GetLocation(), targetPlayer->GetLocation(), hitInfo.range))
						{
							hitResult.HitUserArray[hitplayerCount++] = targetPlayer;
						}
						break;
					}

					if (hitInfo.MaxUserCount <= hitplayerCount || hitplayerCount >= ClientAttack::MaxUserCount)
					{
						hitstopplayer = true;
						break;
					}
				}
			}

			if (hitInfo.bHitMonster && !hitstopmonster)
			{
				int count = m_sectors[sy][sx].GetMonsterCount();
				for (int i = 0; i < count; i++)
				{
					CMonster* targetMonster = m_sectors[sy][sx].GetMonster(i);

					switch (hitInfo.shape)
					{
					case EHitShape::Cone:
						if (AttackCollision::IsInCone(attacker->GetLocation(), targetMonster->GetLocation(), hitInfo.range, hitInfo.attackYaw, hitInfo.halfAngleDegree))
						{
							hitResult.HitMonsterArray[hitmonsterCount++] = targetMonster;
						}
						break;

					case EHitShape::Circle:
						if (AttackCollision::IsInCircle(attacker->GetLocation(), targetMonster->GetLocation(), hitInfo.range))
						{
							hitResult.HitMonsterArray[hitmonsterCount++] = targetMonster;
						}
						break;
					}
				}

				if (hitInfo.MaxMonsterCount <= hitmonsterCount || hitmonsterCount >= ClientAttack::MaxMonsterCount)
				{
					hitstopmonster = true;
					break;
				}
			}

			if (hitstopplayer && hitstopmonster)
			{
				hitResult.HitUserCount = hitplayerCount;
				hitResult.HitMonsterCount = hitmonsterCount;
				return;
			}
		}
	}

	hitResult.HitUserCount = hitplayerCount;
	hitResult.HitMonsterCount = hitmonsterCount;

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

	pUser->SetMoveYaw(movementyaw);
	pUser->SetMoveFlag(moveflag);

	// 싱크 틀어졌으면 싱크 패킷 및 input Update 패킷 보내기
	if (std::abs(pUser->GetX() - xpos) >= SYNC_X_RANGE || std::abs(pUser->GetY() - ypos) >= SYNC_Y_RANGE)
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


		CMessage* pSyncMyChrMsg = PacketBuilder::SyncMyCharacter(pUser);
		SendPacket(pUser->GetSessionID(), pSyncMyChrMsg);
		CMessage::Free(pSyncMyChrMsg);


		CMessage* pSyncOthrChrMsg = PacketBuilder::SyncOtherCharacter(pUser);
		SendPacket_SectorAround(pSyncMyChrMsg, pUser);
		CMessage::Free(pSyncOthrChrMsg);

		syncCount++;
	}
	else
	{
		// 싱크 안틀어졌으면 클라의 좌표를 서버가 믿어줌.
		Location loc{ xpos, ypos, zpos };
		pUser->SetLocation(loc);

		uint16 newSecX = (xpos - MAP_WORLD_OFFSET_X) / SECTOR_SIZE;
		uint16 newSecY = (ypos - MAP_WORLD_OFFSET_Y) / SECTOR_SIZE;

		SectorPos newSec(newSecX, newSecY);
		// 변경된 좌표에 해당하는 섹터가 기존 섹터 좌표와 다르면 섹터 업데이트
		SectorUpdate(pUser, newSec);
	}


	// Movement Update 패킷 뿌리기
	CMessage* pInputUpdateMsg = PacketBuilder::UpdateCharacterMovement(pUser);
	SendPacket_SectorAround(pInputUpdateMsg, pUser);
	CMessage::Free(pInputUpdateMsg);

}

void FieldGroup::HandleRTTMessage(uint64 sessionID, CMessage* pMessage)
{
	double recvtime;

	*pMessage >> recvtime;

	CMessage* pRTTMessage = PacketBuilder::CreateRTTEchoMessage();
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

	if (!pUser->CanSwing(timeGetTime(), swingindex))
	{
		Disconnect(sessionID);
		return;
	}

	// 공격자의 위치, 공격 타입을 매개인자로 전달하여 피격자들 찾기
	HitSearchInfo info;
	HitSearchBuilder::MakeBaseAttack(pUser, attackyaw, info);

	HitResult result;
	CollectHitTarget(pUser,info,result);

	uint32 curTime = timeGetTime();

	// 데미지 계산 및 hp 수정
	for (int i = 0; i < result.HitUserCount; i++)
	{
		uint32 damage = pUser->CalBaseAttackDamage(result.HitUserArray[i], curTime);
		result.HitUserArray[i]->Damage(damage);
		if (result.HitUserArray[i]->GetHP() <= 0 && result.HitUserArray[i]->GetDisconnectFlag())
		{
			Disconnect(sessionID);
			result.HitUserArray[i]->SetDisconnectFlag(true);
		}
	}

	// 공격자 swing 메세지 뿌리기 
	CMessage* pSwingMsg = PacketBuilder::AttackLeftSwing(pUser,attackyaw);
	SendPacket_SectorAround(pSwingMsg, pUser);
	CMessage::Free(pSwingMsg);


	// 피격자 피격 메세지 뿌리기
	CMessage* pHitMsg = PacketBuilder::HitTarget(result.HitUserCount, result.HitMonsterCount,result.HitUserArray, result.HitMonsterArray);

	SectorPos sendflagArray[5];
	int pushCount = 0;

	for (int i = 0; i < result.HitUserCount; i++)
	{
		uint16 secX = result.HitUserArray[i]->GetSectorXpos();
		uint16 secY = result.HitUserArray[i]->GetSectorYpos();

		// 이미 메세지 넣은 섹터 좌표면 pass
		if (SectorPos::IsAlreadyPushed(sendflagArray, pushCount, secX, secY))
			continue;

		SendPacket_SectorAround(pHitMsg, result.HitUserArray[i], true);
		sendflagArray[pushCount++] = SectorPos{ secX , secY };
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

	pUser->SwingStop();

	// 공격자 스윙 정지 메세지 뿌리기
	CMessage* pSwingStop = PacketBuilder::StopLeftSwing(pUser);
	SendPacket_SectorAround(pSwingStop, pUser);
	CMessage::Free(pSwingStop);
}

void FieldGroup::HandleSkillUse(uint64 sessionID, CMessage* pMessage)
{
	uint8 skillslot;
	*pMessage >> skillslot;

	if (skillslot < 0 || skillslot >= UserConst::USER_SKILL_SLOT_COUNT)
	{
		Disconnect(sessionID);
		return;
	}

	uint32 curTime = timeGetTime();

	CUser* pUser = nullptr;
	std::unordered_map<uint64, CUser*>::iterator it = m_userLookUpTable.find(sessionID);
	if (it == m_userLookUpTable.end())
		__debugbreak();

	pUser = it->second;

	bool Success = false;

	if (pUser->CanUseSkill(curTime, skillslot))
	{
		Success = true;
		pUser->UseSkill(curTime, skillslot);
	}

	// 스킬 성공 여부 패킷 스킬 사용자에게 전달
	CMessage* pUseSkillRes = PacketBuilder::UseSkillRes(skillslot, Success);
	SendPacket(sessionID, pUseSkillRes);
	CMessage::Free(pUseSkillRes);


	// 스킬 성공했으면 주변에 뿌리기
	if (Success)
	{
		// Skill 사용 패킷 뿌리기
		CMessage* pUseSkillBroad = PacketBuilder::UseSkillBroadCast(sessionID, skillslot);
		SendPacket_SectorAround(pUseSkillBroad, pUser);
		CMessage::Free(pUseSkillBroad);
	}

	// 스킬이 버프면 여기서 리턴, 액티브 스킬이면 피격 판단
	if (skillslot < UserConst::USER_BUFF_SKILL_SLOT_COUNT)
		return;

	HitSearchInfo info;
	HitResult result;
	HitSearchBuilder::MakeSkillAttack(pUser, skillslot, 0, info);

	CollectHitTarget(pUser, info, result);

	// 데미지 계산 및 hp 수정
	for (int i = 0; i < result.HitUserCount; i++)
	{
		uint32 damage = pUser->CalBaseAttackDamage(result.HitUserArray[i], curTime);
		result.HitUserArray[i]->Damage(damage);
	}

	// 피격자 피격 메세지 뿌리기
	CMessage* pHitMsg = PacketBuilder::HitTarget(result.HitUserCount, result.HitMonsterCount, result.HitUserArray, result.HitMonsterArray);

	SectorPos sendflagArray[5];
	int pushCount = 0;

	for (int i = 0; i < result.HitUserCount; i++)
	{
		uint16 secX = result.HitUserArray[i]->GetSectorXpos();
		uint16 secY = result.HitUserArray[i]->GetSectorYpos();

		// 이미 메세지 넣은 섹터 좌표면 pass
		if (SectorPos::IsAlreadyPushed(sendflagArray, pushCount, secX, secY))
			continue;

		SendPacket_SectorAround(pHitMsg, result.HitUserArray[i], true);
		sendflagArray[pushCount++] = SectorPos{ secX , secY };
	}

	CMessage::Free(pHitMsg);
}

void FieldGroup::MovementProc()
{
	std::unordered_map<uint64, CUser*>::iterator it = m_userLookUpTable.begin();

	for (; it != m_userLookUpTable.end(); ++it)
	{
		CUser* pUser = it->second;

		if (pUser->Move())
			continue;


		uint16 newSectorXpos = (pUser->GetX() - MAP_WORLD_OFFSET_X) / SECTOR_SIZE;
		uint16 newSectorYpos = (pUser->GetY() - MAP_WORLD_OFFSET_Y) / SECTOR_SIZE;

		const SectorPos& newSec{ newSectorXpos, newSectorYpos };

		SectorUpdate(pUser, newSec);
	}
}

void FieldGroup::SectorUpdate(CUser* pUser, const SectorPos& newSec)
{
	// 같은 섹터 좌표면 리턴
	if (SectorPos::SameSector(pUser->GetSectorPos(),newSec))
		return;

	// 현재 섹터에서 삭제 작업
	FieldSector& oldSector = m_sectors[pUser->GetSectorYpos()][pUser->GetSectorXpos()];
	oldSector.RemoveUser(pUser);

	FieldSector& newSector = m_sectors[newSec.GetY()][newSec.GetX()];
	newSector.AddUser(pUser);


	// 새롭게 보이는 섹터에 캐릭터 생성 메세지, 안보이는 섹터에 캐릭터 삭제 메세지 보내기
	SectorAround DeleteSector;
	SectorAround CreateSector;
	SectorPos::CalSectorTransitionMessageTargets(pUser->GetSectorPos(), newSec, DeleteSector, CreateSector);

	// 캐릭터 삭제 메세지 보내기
	CMessage* pDeleteMsg = PacketBuilder::DeleteCharacter(pUser);

	for (int i = 0; i < DeleteSector.m_count; i++)
	{
		SendPacket_SectorOne(pDeleteMsg, DeleteSector.m_Around[i].GetX(), DeleteSector.m_Around[i].GetY(), pUser);
	}
	CMessage::Free(pDeleteMsg);


	// 캐릭터 생성 메세지 보내기
	CMessage* pCreateMsg = PacketBuilder::CreateOtherCharacter(pUser);
	for (int i = 0; i < CreateSector.m_count; i++)
	{
		SendPacket_SectorOne(pCreateMsg, CreateSector.m_Around[i].GetX(), CreateSector.m_Around[i].GetY(), pUser);
	}
	CMessage::Free(pCreateMsg);

	pUser->SetNewSectorPos(newSec);
}

void FieldGroup::UserManaRegen()
{
	uint32 curTime = timeGetTime();

	if (curTime - m_ManaRegenOldTime < 1000)
		return;

	CUser* targetUser = nullptr;
	std::unordered_map<uint64, CUser*>::iterator it;

	for (it = m_userLookUpTable.begin(); it != m_userLookUpTable.end(); ++it)
	{
		it->second->ManaRegen(curTime);
	}
}

void FieldGroup::MonsterSpawnInit()
{
	GrossMonsterSpawnInit();
}

void FieldGroup::GrossMonsterSpawnInit()
{
	// 최소 섹터당 몬스터 1마리 배치
	if (MAX_GROSS_FIELD_MONSTER_COUNT < GROSS_FIELD_SECTOR_COUNT)
		return;

	int monstrSpawnCount = 0;
	for (int i = 0; i < GROSS_FIELD_AREA_ARRAY_COUNT; i++)
	{
		for (uint16 sy = GROSS_FIELD_MONSTER_SPAWN_AREAS[i].minSectorY; sy <= GROSS_FIELD_MONSTER_SPAWN_AREAS[i].maxSectorY; sy++)
		{
			for (uint16 sx = GROSS_FIELD_MONSTER_SPAWN_AREAS[i].minSectorX; sx <= GROSS_FIELD_MONSTER_SPAWN_AREAS[i].maxSectorX; sx++)
			{
				for (int count = 0; count < MAX_GROSS_FIELD_MONSTER_COUNT / GROSS_FIELD_SECTOR_COUNT; count++)
				{
					if (monstrSpawnCount >= MAX_GROSS_FIELD_MONSTER_COUNT)
						return;

					float xpos = MAP_WORLD_OFFSET_X + sx * SECTOR_SIZE + (rand() / SECTOR_SIZE + 10);
					float ypos = MAP_WORLD_OFFSET_Y + sy * SECTOR_SIZE + (rand() / SECTOR_SIZE + 10);


					CMonster& monster = m_grossMonsterPoolArray[monstrSpawnCount++];
					Location loc{ MAP_WORLD_OFFSET_X + (sx + 0.5f) * SECTOR_SIZE,MAP_WORLD_OFFSET_Y + (sy + 0.5f) * SECTOR_SIZE ,-38690.f };
					monster.Init(m_monsterAllocID, 0, loc);
					m_monsterAllocID++;

					m_sectors[sy][sx].AddMonster(&monster);

				}
			}
		}
	}
}


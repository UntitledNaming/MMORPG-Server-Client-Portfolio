#include <windows.h>
#include <vector>
#include <unordered_map>
#include <array>
#include <cmath>
#include <chrono>
#include <set>
#include "CMonster.h"
#include "ContentsDefine.h"
#include "ContentsEnum.h"
#include "ContentsProtocol.h"
#include "MemoryPoolTLS.h"
#include "CMessage.h"
#include "CGroup.h"
#include "PacketBuilder.h"
#include "CollisionCheck.h"
#include "FieldSector.h"
#include "SectorPos.h"
#include "Inventory.h"
#include "Equipment.h"
#include "QuickSlot.h"
#include "CUserItemStorage.h"
#include "IUser.h"
#include "CUser.h"
#include "HitSearchBuilder.h"
#include "ItemTable.h"
#include "FieldDropItemPool.h"
#include "LatencyHistogram.h"
#include "FieldGroup.h"


using namespace FieldConst;
using namespace FieldProtocol;

size_t FieldGroup::UserCount()
{
	return m_userLookUpTable.size();
}

void FieldGroup::InitDBManager(CDBManager* pDBManager)
{
	m_DBManagerPtr = pDBManager;
}

void FieldGroup::SendMonsterCreateToSector(CMonster* pMonster, uint16 secX, uint16 secY)
{
	// 섹터에 있는 유저들에게 몬스터 생성 및 필요하면 Move 패킷 보내기
	auto start = std::chrono::steady_clock::now();

	int count = m_sectors[secY][secX].GetUserCount();


	CMessage* pCreateMonster = PacketBuilder::CreateMonster(pMonster);
	CMessage* pMonsterMove = PacketBuilder::MoveMonster(pMonster, pMonster->GetMonsterAITargetLocation());

	for (int i = 0; i < count; i++)
	{
		CUser* pUser = m_sectors[secY][secX].GetUser(i);

		SendPacket(pUser->GetSessionID(), pCreateMonster);

		// 순찰, 추격, 복귀 중일 때 Move 패킷 보내기.
		if (pMonster->GetMonsterState() == EMonsterState::Patrol || pMonster->GetMonsterState() == EMonsterState::Chase || pMonster->GetMonsterState() == EMonsterState::Return)
		{
			SendPacket(pUser->GetSessionID(), pMonsterMove);
		}

	}
	CMessage::Free(pCreateMonster);
	CMessage::Free(pMonsterMove);

	auto end = std::chrono::steady_clock::now();

	m_BroadCastProcTime[(int)BroadCastType::CreateMonster].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
}

void FieldGroup::SendMonsterDeleteToSector(CMonster* pMonster, uint16 secX, uint16 secY)
{
	auto start = std::chrono::steady_clock::now();

	int count = m_sectors[secY][secX].GetUserCount();
	CMessage* pDeleteMonster = PacketBuilder::DeleteMonster(pMonster);
	for (int i = 0; i < count; i++)
	{
		CUser* pUser = m_sectors[secY][secX].GetUser(i);


		SendPacket(pUser->GetSessionID(), pDeleteMonster);
	}

	CMessage::Free(pDeleteMonster);

	auto end = std::chrono::steady_clock::now();
	m_BroadCastProcTime[(int)BroadCastType::DeleteMonster].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
}

void FieldGroup::SendMonsterTargetUpdate(CMonster* pMonster)
{
	SectorAround MoveAround;
	SectorPos::SectorFind(MoveAround, pMonster->GetSectorPos());
	CMessage* pMoveMonster = PacketBuilder::MoveMonster(pMonster, pMonster->GetMonsterAITargetLocation());

	for (int i = 0; i < MoveAround.m_count; i++)
	{
		SendPacket_SectorOne(BroadCastType::MoveMonster,pMoveMonster, MoveAround.m_Around[i].GetX(), MoveAround.m_Around[i].GetY(), nullptr);
	}

	CMessage::Free(pMoveMonster);
}

void FieldGroup::SendMonsterAttackTarget(CMonster* pMonster, CUser* pTarget, int16 newHP)
{
	// 피격 대상인 클라에게는 hp 담아서 보내기
	CMessage* pAttackMonsterToMe = PacketBuilder::AttackMonsterToMe(pMonster, pTarget->GetSessionID(), newHP);
	SendPacket(pTarget->GetSessionID(), pAttackMonsterToMe);
	CMessage::Free(pAttackMonsterToMe);


	// 피격 대상 주변 클라 들에게는 피격 대상의 newRatio만 보내기
	CMessage* pAttackMonsterToOther = PacketBuilder::AttackMonsterToOther(pMonster, pTarget);

	// 몬스터와 타겟 주변 섹터에 해당 메세지 뿌리기(타겟 때리는 애니 + 피격 대상 hp 깎기 같이 나감)
	SectorPos sendflagArray[20];
	int pushCount = 0;

	// 몬스터와 타겟 주변 섹터 찾기
	SectorAround monsterAround;
	SectorAround targetAround;
	SectorPos::SectorFind(monsterAround, pMonster->GetSectorPos());
	SectorPos::SectorFind(targetAround, pTarget->GetSectorPos());

	for (int i = 0; i < monsterAround.m_count; i++)
	{
		int16 secX = monsterAround.m_Around[i].GetX();
		int16 secY = monsterAround.m_Around[i].GetY();

		SendPacket_SectorOne(BroadCastType::MonsterHitPlayer, pAttackMonsterToOther, secX, secY, pTarget);
		sendflagArray[pushCount++] = SectorPos{ secX , secY };
	}

	for (int i = 0; i < targetAround.m_count; i++)
	{
		int16 secX = targetAround.m_Around[i].GetX();
		int16 secY = targetAround.m_Around[i].GetY();

		// 이미 메세지 넣은 섹터 좌표면 pass
		if (SectorPos::IsAlreadyPushed(sendflagArray, pushCount, secX, secY))
			continue;

		SendPacket_SectorOne(BroadCastType::MonsterHitPlayer,pAttackMonsterToOther, secX, secY, pTarget);
		sendflagArray[pushCount++] = SectorPos{ secX , secY };
	}

	CMessage::Free(pAttackMonsterToOther);
}

void FieldGroup::SendMonsterStop(CMonster* pMonster)
{
	CMessage* pStopMonster = PacketBuilder::StopMonster(pMonster, pMonster->GetLocation());

	SectorAround StopAround;
	SectorPos::SectorFind(StopAround, pMonster->GetSectorPos());

	for (int i = 0; i < StopAround.m_count; i++)
	{
		SendPacket_SectorOne(BroadCastType::StopMonster, pStopMonster, StopAround.m_Around[i].GetX(), StopAround.m_Around[i].GetY(), nullptr);
	}
	CMessage::Free(pStopMonster);
}

void FieldGroup::SendCreateFieldDropItem(FieldDropItem* pItem)
{
	CMessage* pCreateFieldDropItem = PacketBuilder::CreateFieldDropItem(pItem);

	SectorAround CreateAround;
	SectorPos::SectorFind(CreateAround, pItem->sectorPos);

	for (int i = 0; i < CreateAround.m_count; i++)
	{
		SendPacket_SectorOne(BroadCastType::CreateFieldDropItem, pCreateFieldDropItem, CreateAround.m_Around[i].GetX(), CreateAround.m_Around[i].GetY(), nullptr);
	}

	CMessage::Free(pCreateFieldDropItem);
}

void FieldGroup::SendDeleteFieldDropItem(FieldDropItem* pItem)
{
	CMessage* pDeleteFieldDropItem = PacketBuilder::DeleteFieldDropItem(pItem);

	SectorAround DeleteAround;
	SectorPos::SectorFind(DeleteAround, pItem->sectorPos);

	for (int i = 0; i < DeleteAround.m_count; i++)
	{
		SendPacket_SectorOne(BroadCastType::DeleteFieldDropItem, pDeleteFieldDropItem, DeleteAround.m_Around[i].GetX(), DeleteAround.m_Around[i].GetY(), nullptr);
	}

	CMessage::Free(pDeleteFieldDropItem);
}

void FieldGroup::AddMonsterToSector(CMonster* pMonster, uint16 secX, uint16 secY)
{
	m_sectors[secY][secX].AddMonster(pMonster);
}

void FieldGroup::RemoveMonsterToSector(CMonster* pMonster, uint16 secX, uint16 secY)
{
	m_sectors[secY][secX].RemoveMonster(pMonster);
}

CUser* FieldGroup::GetUser(uint64 sessionID)
{
	CUser* pUser = nullptr;
	std::unordered_map<uint64, CUser*>::iterator it = m_userLookUpTable.begin();
	it = m_userLookUpTable.find(sessionID);

	if (it == m_userLookUpTable.end())
		return nullptr;
	
	pUser = it->second;
	return pUser;
}

void FieldGroup::Init(CGameLibrary* p)
{
	srand(static_cast<unsigned int>(time(nullptr)));

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
			m_sectors[y][x].Init();
		}
	}

	// 섹터에 몬스터 배치하기
	MonsterSpawnInit();
}

void FieldGroup::Destroy()
{
	// 현재 있는 모든 유저 연결 끊기
	std::unordered_map<uint64, CUser*>::iterator it = m_userLookUpTable.begin();
	for (; it != m_userLookUpTable.end(); ++it)
	{
		Disconnect(it->second->GetSessionID());
	}

	// 유저 전부 삭제될때까지 대기
	while (!m_userLookUpTable.empty())
	{

	}


	for (int i = 0; i < MAX_GROSS_FIELD_MONSTER_COUNT; i++)
	{
		m_grossMonsterPoolArray[i].Destroy();
	}
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
	SendPacket_SectorAround(BroadCastType::DeleteCharacter,pMessage, pUser);
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

	case PACKET_CS_USE_SKILL:
		HandleSkillUse(sessionID, pMessage);
		break;

	case PACKET_CS_PICK_UP_ITEM:
		HandlePickUpItems(sessionID, pMessage);
		break;

	case PACKET_CS_USE_ITEM:
		HandleUseItem(sessionID, pMessage);
		break;

	case PACKET_CS_DELETE_ITEM:
		HandleDeleteItem(sessionID, pMessage);
		break;

	case PACKET_CS_SWAP_SLOT:
		HandleSwapSlot(sessionID, pMessage);
		break;

	case PACKET_CS_RESPAWN_REQ:
		HandleRespawn(sessionID, pMessage);
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
	SectorPos::SectorFind(sectAround, pOnUser->GetSectorPos());

	// 섹터 순회하면서 생성 관련 메세지 보내기
	for (int i = 0; i < sectAround.m_count; i++)
	{
		uint16 curSecXpos = sectAround.m_Around[i].GetX();
		uint16 curSecYpos = sectAround.m_Around[i].GetY();

		// 주변 섹터에 본인 캐릭터 생성 메세지 만들고 보내기
		CMessage* pCreateMyChrToOtherMsg = PacketBuilder::CreateOtherCharacter(pOnUser);
		SendPacket_SectorOne(BroadCastType::CreateMyCharacterToOther,pCreateMyChrToOtherMsg, curSecXpos, curSecYpos, pOnUser);
		CMessage::Free(pCreateMyChrToOtherMsg);

		// 해당 섹터의 유저 생성 메세지를 만들어 본인 캐릭터에게 전송
		uint16 curUserCount = m_sectors[curSecYpos][curSecXpos].GetUserCount();

		// 섹터에 있는  타 유저 순회
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
			CMonster* pMonster = m_sectors[curSecYpos][curSecXpos].GetMonster(monstercount);

			// 죽은 몬스터면 생성 Pass
			if (pMonster->GetMonsterState() == EMonsterState::Dead)
				continue;

			CMessage* pCreateMonsterMsg = PacketBuilder::CreateMonster(pMonster);
			SendPacket(pOnUser->GetSessionID(), pCreateMonsterMsg);
			CMessage::Free(pCreateMonsterMsg);


			// 순찰, 추격, 복귀 중일 때 Move 패킷 보내기.
			if (pMonster->GetMonsterState() == EMonsterState::Patrol || pMonster->GetMonsterState() == EMonsterState::Chase || pMonster->GetMonsterState() == EMonsterState::Return)
			{
				CMessage* pMonsterMove = PacketBuilder::MoveMonster(pMonster, pMonster->GetMonsterAITargetLocation());
				SendPacket(pOnUser->GetSessionID(), pMonsterMove);
				CMessage::Free(pMonsterMove);
			}
		}

		// 주변 섹터에 있는 아이템에 대한 생성 메세지 보내기
		uint16 curItemCount = m_sectors[curSecYpos][curSecXpos].GetItemCount();
		for (int itemcount = 0; itemcount < curItemCount; itemcount++)
		{
			CMessage* pCreateItemMsg = PacketBuilder::CreateFieldDropItem(m_sectors[curSecYpos][curSecXpos].GetFieldDropItem(itemcount));
			SendPacket(pOnUser->GetSessionID(), pCreateItemMsg);
			CMessage::Free(pCreateItemMsg);
		}
	}
}

void FieldGroup::OnUpdate(long long LockEnterTime)
{
	m_OnUpdateLockEnterTime.Record(LockEnterTime);

	auto start = std::chrono::steady_clock::now();

	UserUpdate();

	auto end1 = std::chrono::steady_clock::now();

	MonsterUpdate();

	auto end2 = std::chrono::steady_clock::now();

	FieldDropItemExpired();

	auto end3 = std::chrono::steady_clock::now();

	m_OnUpdateProcTime[(int)FrameProcType::Whole].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end3 - start).count());
	m_OnUpdateProcTime[(int)FrameProcType::User].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end1 - start).count());
	m_OnUpdateProcTime[(int)FrameProcType::Monster].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end2 - end1).count());
	m_OnUpdateProcTime[(int)FrameProcType::FieldDropItem].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end3 - end2).count());

	InterlockedIncrement(&m_FrameTPS);
}

void FieldGroup::SendPacket_SectorOne(BroadCastType type, CMessage* pMessage, uint16 xpos, uint16 ypos, CUser* pUser)
{
	uint16 count = m_sectors[ypos][xpos].GetUserCount();

	auto start = std::chrono::steady_clock::now();

	for (int i = 0; i < count; i++)
	{
		CUser* pCurUser = m_sectors[ypos][xpos].GetUser(i);

		// 매개인자로 받은 유저와 같은 유저면 메세지 송신 Pass
		if (pCurUser == pUser)
			continue;

	    SendPacket(pCurUser->GetSessionID(), pMessage);
	}
	auto end = std::chrono::steady_clock::now();

	m_BroadCastProcTime[(int)type].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
}

void FieldGroup::SendPacket_SectorAround(BroadCastType type, CMessage* pMessage, CUser* pUser, bool userSend)
{
	SectorAround around;
	SectorPos::SectorFind(around, pUser->GetSectorPos());

	for (int i = 0; i < around.m_count; i++)
	{
		if (userSend)
		{
			SendPacket_SectorOne(type, pMessage, around.m_Around[i].GetX(), around.m_Around[i].GetY(), nullptr);
			continue;
		}

		SendPacket_SectorOne(type, pMessage, around.m_Around[i].GetX(), around.m_Around[i].GetY(), pUser);
	}
}

void FieldGroup::SendPacket_HitSectors(HitResult& result)
{
	// 피격자 피격 메세지 뿌리기
	CMessage* pHitMsg = PacketBuilder::HitTarget(result.HitUserCount, result.HitMonsterCount, result.HitUserArray, result.HitMonsterArray);

	SectorPos sendflagArray[20];
	int pushCount = 0;


	// 피격자 들 순회하면서 피격자 섹터에 있는 사람들에게 피격 메세지 전달하기
	for (int i = 0; i < result.HitUserCount; i++)
	{
		int16 secX = result.HitUserArray[i]->GetSectorXpos();
		int16 secY = result.HitUserArray[i]->GetSectorYpos();

		// 피격자 주변 섹터 좌표 찾기
		SectorAround HitAround;
		SectorPos::SectorFind(HitAround, result.HitUserArray[i]->GetSectorPos());

		for (int count = 0; count < HitAround.m_count; count++)
		{
			int16 hitSecX = HitAround.m_Around[count].GetX();
			int16 hitSecY = HitAround.m_Around[count].GetY();

			// 이미 메세지 넣은 섹터 좌표면 pass
			if (SectorPos::IsAlreadyPushed(sendflagArray, pushCount, hitSecX, hitSecY))
				continue;

			SendPacket_SectorOne(BroadCastType::AttackHitResult, pHitMsg, hitSecX, hitSecY, nullptr);

			sendflagArray[pushCount++] = SectorPos{ hitSecX , hitSecY };
		}


	}

	// 피격몬스터들 주변 섹터에 메세지 뿌리기
	for (int i = 0; i < result.HitMonsterCount; i++)
	{
		SectorAround HitAround;

		// 피격 몬스터 주변 섹터 찾기
		SectorPos::SectorFind(HitAround, result.HitMonsterArray[i]->GetSectorPos());

		for (int count = 0; count < HitAround.m_count; count++)
		{
			// 피격 몬스터 주변 섹터 좌표 얻기
			int16 hitSecX = HitAround.m_Around[count].GetX();
			int16 hitSecY = HitAround.m_Around[count].GetY();

			// 이미 메세지 넣은 섹터 좌표면 pass
			if (SectorPos::IsAlreadyPushed(sendflagArray, pushCount, hitSecX, hitSecY))
				continue;

			SendPacket_SectorOne(BroadCastType::AttackHitResult, pHitMsg, hitSecX, hitSecY, nullptr);

			sendflagArray[pushCount++] = SectorPos{ hitSecX , hitSecY };
		}
	}
	CMessage::Free(pHitMsg);
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

					// 타겟이 공격자와 같으면 pass or 타겟이 죽었으면 pass
					if (targetPlayer == attacker || targetPlayer->IsAlive() <= 0 )
						continue;

					switch (hitInfo.shape)
					{
					case EHitShape::Cone:
						if (CollisionCheck::IsInCone(attacker->GetLocation(), targetPlayer->GetLocation(), hitInfo.range, hitInfo.attackYaw, hitInfo.halfAngleDegree))
						{
							hitResult.HitUserArray[hitplayerCount++] = targetPlayer;
						}
						break;

					case EHitShape::Circle:
						if (CollisionCheck::IsInCircle(attacker->GetLocation(), targetPlayer->GetLocation(), hitInfo.range))
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

					if (targetMonster->GetMonsterState() == EMonsterState::Dead)
						continue;

					switch (hitInfo.shape)
					{
					case EHitShape::Cone:
						if (CollisionCheck::IsInCone(attacker->GetLocation(), targetMonster->GetLocation(), hitInfo.range, hitInfo.attackYaw, hitInfo.halfAngleDegree))
						{
							hitResult.HitMonsterArray[hitmonsterCount++] = targetMonster;
						}
						break;

					case EHitShape::Circle:
						if (CollisionCheck::IsInCircle(attacker->GetLocation(), targetMonster->GetLocation(), hitInfo.range))
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
	m_OnRecvLockEnterTime.Record(pMessage->m_recvLockWaits);
	moveReqCount++;
	auto start = std::chrono::steady_clock::now();

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
	if(!pUser->IsAlive())
		return;


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
			//Disconnect(sessionID);
			return;
		}


		CMessage* pSyncMyChrMsg = PacketBuilder::SyncMyCharacter(pUser);
		SendPacket(pUser->GetSessionID(), pSyncMyChrMsg);
		CMessage::Free(pSyncMyChrMsg);

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
	SendPacket_SectorAround(BroadCastType::UpdateCharacterMovementInput, pInputUpdateMsg, pUser);
	CMessage::Free(pInputUpdateMsg);


	auto end = std::chrono::steady_clock::now();
	m_OnRecvMsgProcTime[(int)FieldRecvType::CharacterMovement].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
}

void FieldGroup::HandleRTTMessage(uint64 sessionID, CMessage* pMessage)
{
	m_OnRecvLockEnterTime.Record(pMessage->m_recvLockWaits);

	auto start = std::chrono::steady_clock::now();

	double recvtime;

	*pMessage >> recvtime;

	CMessage* pRTTMessage = PacketBuilder::CreateRTTEchoMessage();
	SendPacket(sessionID, pRTTMessage);
	CMessage::Free(pRTTMessage);

	auto end = std::chrono::steady_clock::now();
	m_OnRecvMsgProcTime[(int)FieldRecvType::RTT].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
}

void FieldGroup::HandleLeftAttackSwing(uint64 sessionID, CMessage* pMessage)
{
	m_OnRecvLockEnterTime.Record(pMessage->m_recvLockWaits);

	leftAttackReqCount++;

	auto start = std::chrono::steady_clock::now();

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
	if (!pUser->IsAlive())
		return;


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

	}

	// 데미지 계산 및 hp 수정
	uint32 totalExp = 0;

	for (int i = 0; i < result.HitMonsterCount; i++)
	{
		uint32 damage = pUser->CalBaseAttackDamage(result.HitMonsterArray[i], curTime);
		result.HitMonsterArray[i]->Damage(damage);

		if (result.HitMonsterArray[i]->IsAlive())
			continue;

		// 아이템 생성 실패시 다음
		CreateFieldDropItem(*result.HitMonsterArray[i]);

		// 경험치 획득량 체크
		totalExp += result.HitMonsterArray[i]->GetExp();
	}

	// 획득한 경험치 한번에 유저에 반영
	GainEXPResult gainResult = {};
	gainResult.levelUp = false;
	if (pUser->GainExp(totalExp, gainResult))
	{
		// 경험치 획득 성공시 패킷 보내기
		CMessage* GainExpMsg = PacketBuilder::GainExp(gainResult);
		SendPacket(sessionID, GainExpMsg);
		CMessage::Free(GainExpMsg);
	}


	// 공격자 swing 메세지 뿌리기 
	CMessage* pSwingMsg = PacketBuilder::AttackLeftSwing(pUser->GetSessionID(), attackyaw, swingindex);
	SendPacket_SectorAround(BroadCastType::LeftSwing, pSwingMsg, pUser);
	CMessage::Free(pSwingMsg);

	SendPacket_HitSectors(result);

	// 피격 몬스터들 중에 죽었으면 삭제 메세지 뿌리고 섹터에서 제거
	for (int i = 0; i < result.HitMonsterCount; i++)
	{
		CMonster* pHitMonster = result.HitMonsterArray[i];

		if (pHitMonster->GetMonsterState() != EMonsterState::Dead)
			continue;

		SectorAround DeleteSector;

		// 피격 몬스터 주변 섹터 찾기
		SectorPos::SectorFind(DeleteSector, pHitMonster->GetSectorPos());

		for (int count = 0; count < DeleteSector.m_count; count++)
		{
			SendMonsterDeleteToSector(pHitMonster, DeleteSector.m_Around[count].GetX(), DeleteSector.m_Around[count].GetY());
		}

		m_sectors[pHitMonster->GetSectorY()][pHitMonster->GetSectorX()].RemoveMonster(pHitMonster);
	}


	auto end = std::chrono::steady_clock::now();
	m_OnRecvMsgProcTime[(int)FieldRecvType::LeftSwing].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());

}

void FieldGroup::HandleSkillUse(uint64 sessionID, CMessage* pMessage)
{
	m_OnRecvLockEnterTime.Record(pMessage->m_recvLockWaits);

	skillUseReqCount++;

	auto start = std::chrono::steady_clock::now();

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
	if (!pUser->IsAlive())
		return;


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
		SendPacket_SectorAround(BroadCastType::UseSkill, pUseSkillBroad, pUser);
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
		uint32 damage = pUser->CalSkillDamage(skillslot, result.HitUserArray[i], curTime);
		result.HitUserArray[i]->Damage(damage);
	}

	// 데미지 계산 및 hp 수정
	uint32 totalExp = 0;

	for (int i = 0; i < result.HitMonsterCount; i++)
	{
		uint32 damage = pUser->CalSkillDamage(skillslot, result.HitMonsterArray[i], curTime);
		result.HitMonsterArray[i]->Damage(damage);

		// 살아있으면 Pass
		if (result.HitMonsterArray[i]->IsAlive())
			continue;

		// 아이템 생성
		CreateFieldDropItem(*result.HitMonsterArray[i]);
		
		// 경험치 획득량 체크
		totalExp += result.HitMonsterArray[i]->GetExp();
	}

	// 획득한 경험치 한번에 유저에 반영
	GainEXPResult gainResult = {};
	gainResult.levelUp = false;
	if (pUser->GainExp(totalExp, gainResult))
	{
		// 경험치 획득 성공시 패킷 보내기
		CMessage* GainExpMsg = PacketBuilder::GainExp(gainResult);
		SendPacket(sessionID, GainExpMsg);
		CMessage::Free(GainExpMsg);
	}

	SendPacket_HitSectors(result);

	// 피격 몬스터들 중에 죽었으면 삭제 메세지 뿌리기
	for (int i = 0; i < result.HitMonsterCount; i++)
	{
		CMonster* pHitMonster = result.HitMonsterArray[i];

		if (pHitMonster->GetMonsterState() != EMonsterState::Dead)
			continue;

		SectorAround DeleteSector;

		// 피격 몬스터 주변 섹터 찾기
		SectorPos::SectorFind(DeleteSector, pHitMonster->GetSectorPos());

		for (int count = 0; count < DeleteSector.m_count; count++)
		{
			SendMonsterDeleteToSector(pHitMonster, DeleteSector.m_Around[count].GetX(), DeleteSector.m_Around[count].GetY());
		}

		m_sectors[pHitMonster->GetSectorY()][pHitMonster->GetSectorX()].RemoveMonster(pHitMonster);
	}

	auto end = std::chrono::steady_clock::now();
	m_OnRecvMsgProcTime[(int)FieldRecvType::SkillUse].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
}

void FieldGroup::HandlePickUpItems(uint64 sessionID, CMessage* pMessage)
{
	m_OnRecvLockEnterTime.Record(pMessage->m_recvLockWaits);

	pickupReqCount++;

	auto start = std::chrono::steady_clock::now();

	CUser* pUser = nullptr;

	std::unordered_map<uint64, CUser*>::iterator it = m_userLookUpTable.find(sessionID);
	if (it == m_userLookUpTable.end())
		__debugbreak();

	pUser = it->second;
	if (!pUser->IsAlive())
		return;


	uint64 dropID;
	*pMessage >> dropID;

	std::unordered_map<uint64, FieldDropItem*>::iterator it2 = m_dropItemLookUpTable.find(dropID);
	if (it2 == m_dropItemLookUpTable.end())
		return;

	FieldDropItem* pItem = it2->second;

	// 해당 아이템을 유저 인벤토리에 넣기 실패하면 그냥 리턴
	const ItemData* itemData = ItemTable::GetItemData(pItem->itemID);
	if (itemData == nullptr)
		return;

	PickUpConsumableResult resultConsume = {};
	PickUpEquipResult      resultEquip = {};
	CMessage* pPickUpMsg = nullptr;


	switch (itemData->itemType)
	{
	case ITEM_TYPE::CONSUMABLE:
	{
		if (!pUser->GetConsumableItem(*pItem, resultConsume))
			return;

		pPickUpMsg = PacketBuilder::PickUpConsumableFieldDropItem(&resultConsume);

		break;
	}
	case ITEM_TYPE::EQUIPMENT:
	{
		if (!pUser->GetEquipmentItem(*pItem, resultEquip))
			return;

		pPickUpMsg = PacketBuilder::PickUpEquipFieldDropItem(&resultEquip);

		break;
	}
	}

	if (pPickUpMsg == nullptr)
		return;

	// 인벤토리에 넣은 아이템의 DropUID 삭제 패킷 보내기
	SendDeleteFieldDropItem(pItem);

	// PickUp 결과 메세지 보내주기
	SendPacket(sessionID, pPickUpMsg);

	CMessage::Free(pPickUpMsg);

	// 인벤토리에 넣었으니 자료구조에서 제거 후 풀에 반납
	m_sectors[pItem->sectorPos.GetY()][pItem->sectorPos.GetX()].RemoveItem(pItem);
	m_dropItemLookUpTable.erase(pItem->dropUID);
	FieldDropItemPool::FreeItem(pItem);


	auto end = std::chrono::steady_clock::now();
	m_OnRecvMsgProcTime[(int)FieldRecvType::PickUpItems].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
}

void FieldGroup::HandleUseItem(uint64 sessionID, CMessage* pMessage)
{
	m_OnRecvLockEnterTime.Record(pMessage->m_recvLockWaits);

	useitemReqCount++;

	auto start = std::chrono::steady_clock::now();

	uint8 type;
	int16 slotIndex;

	*pMessage >> type;
	*pMessage >> slotIndex;

	std::unordered_map<uint64, CUser*>::iterator it = m_userLookUpTable.find(sessionID);
	if (it == m_userLookUpTable.end())
		return;

	CUser* pUser = it->second;
	if (!pUser->IsAlive())
		return;

	// 유저 함수 호출 및 결과 구조체 레퍼런스 전달
	UseItemResult result;
	result.success = false;
	result.resultType = USE_ITEM_RESULT::NONE;
	result.consumableResult = {};
	result.unEquipResult = {};
	result.consumableResult = {};

	switch (static_cast<SLOT_TYPE>(type))
	{
	case SLOT_TYPE::EQUIPMENT:
	{
		result.success = pUser->UseEquipmentItem(slotIndex, result);
		break;
	}

	case SLOT_TYPE::INVENTORY:
	{
		result.success = pUser->UseInventoryItem(slotIndex, result);
		break;
	}

	case SLOT_TYPE::QUICKSLOT:
	{
		result.success = pUser->UseQuickSlotItem(slotIndex, result);
		break;
	}

	default:
		Disconnect(sessionID);
		return;
	}

	// 응답 패킷 보내기
	CMessage* pUseItem = PacketBuilder::UseItem(result);
	SendPacket(sessionID, pUseItem);
	CMessage::Free(pUseItem);

	auto end = std::chrono::steady_clock::now();
	m_OnRecvMsgProcTime[(int)FieldRecvType::UseItem].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
}

void FieldGroup::HandleDeleteItem(uint64 sessionID, CMessage* pMessage)
{
	m_OnRecvLockEnterTime.Record(pMessage->m_recvLockWaits);

	deleteitemReqCount++;

	auto start = std::chrono::steady_clock::now();

	uint8 type;
	int16 slotIndex;

	*pMessage >> type;
	*pMessage >> slotIndex;

	std::unordered_map<uint64, CUser*>::iterator it = m_userLookUpTable.find(sessionID);
	if (it == m_userLookUpTable.end())
		return;

	CUser* pUser = it->second;
	if (!pUser->IsAlive())
		return;

	// 유저에 함수 호출
	bool Success = false;
	Success = pUser->DeleteItem(slotIndex, static_cast<SLOT_TYPE>(type));

	// 응답 패킷 보내기
	CMessage* pDeleteItemMsg = PacketBuilder::DeleteItem(Success);
	SendPacket(sessionID, pDeleteItemMsg);
	CMessage::Free(pDeleteItemMsg);

	auto end = std::chrono::steady_clock::now();
	m_OnRecvMsgProcTime[(int)FieldRecvType::DeleteItem].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
}

void FieldGroup::HandleSwapSlot(uint64 sessionID, CMessage* pMessage)
{
	m_OnRecvLockEnterTime.Record(pMessage->m_recvLockWaits);

	swapitemReqCount++;

	auto start = std::chrono::steady_clock::now();

	uint8 fromtype;
	uint8 totype;
	int16 fromslotIndex;
	int16 toslotIndex;

	*pMessage >> fromtype;
	*pMessage >> fromslotIndex;
	*pMessage >> totype;
	*pMessage >> toslotIndex;

	std::unordered_map<uint64, CUser*>::iterator it = m_userLookUpTable.find(sessionID);
	if (it == m_userLookUpTable.end())
		return;

	CUser* pUser = it->second;
	if (!pUser->IsAlive())
		return;


	// 유저에 함수 호출
	bool Success = pUser->ItemSlotChange(static_cast<SLOT_TYPE>(fromtype), fromslotIndex, static_cast<SLOT_TYPE>(totype), toslotIndex);

	// 응답 패킷 보내기
	CMessage* pSwapItem = PacketBuilder::SwapSlot(Success);
	SendPacket(sessionID, pSwapItem);
	CMessage::Free(pSwapItem);

	auto end = std::chrono::steady_clock::now();
	m_OnRecvMsgProcTime[(int)FieldRecvType::SwapItem].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
}

void FieldGroup::HandleRespawn(uint64 sessionID, CMessage* pMessage)
{
	m_OnRecvLockEnterTime.Record(pMessage->m_recvLockWaits);

	rewspawnReqCount++;

	auto start = std::chrono::steady_clock::now();


	std::unordered_map<uint64, CUser*>::iterator it = m_userLookUpTable.find(sessionID);
	if (it == m_userLookUpTable.end())
		return;

	CUser* pUser = it->second;
	if (pUser->IsAlive())
		return;

	// 리스폰 처리
	pUser->ResPawn();

	// 본인에게 메세지 보내기
	CMessage* respawnToMeMsg = PacketBuilder::RespawnToMe(pUser->GetHP(), pUser->GetMP());
	SendPacket(sessionID, respawnToMeMsg);
	CMessage::Free(respawnToMeMsg);

	// 주변에게 뿌리기
	CMessage* respawnToOtherMsg = PacketBuilder::RespawnToOther(pUser->GetSessionID());
	SendPacket_SectorAround(BroadCastType::Respawn, respawnToOtherMsg, pUser);
	CMessage::Free(respawnToOtherMsg);


	auto end = std::chrono::steady_clock::now();
	m_OnRecvMsgProcTime[(int)FieldRecvType::Respawn].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
}

void FieldGroup::UserUpdate()
{
	std::unordered_map<uint64, CUser*>::iterator it = m_userLookUpTable.begin();
	uint32 curTime = timeGetTime();
	for (; it != m_userLookUpTable.end(); ++it)
	{
		CUser* pUser = it->second;

		// 이동 하면 true 리턴
		if (!pUser->UserOnUpdate(curTime))
			continue;


		int16 newSectorXpos = (pUser->GetX() - MAP_WORLD_OFFSET_X) / SECTOR_SIZE;
		int16 newSectorYpos = (pUser->GetY() - MAP_WORLD_OFFSET_Y) / SECTOR_SIZE;

		const SectorPos& newSec{ newSectorXpos, newSectorYpos };

		SectorUpdate(pUser, newSec);
	}
}

void FieldGroup::SectorUpdate(CUser* pUser, const SectorPos& newSec)
{
	if (!SectorPos::SectorRangeCheck(newSec))
		return;

	// 같은 섹터 좌표면 리턴
	if (SectorPos::SameSector(pUser->GetSectorPos(),newSec))
		return;

	// 현재 섹터에서 삭제 작업
	FieldSector& oldSector = m_sectors[pUser->GetSectorYpos()][pUser->GetSectorXpos()];
	oldSector.RemoveUser(pUser);

	FieldSector& newSector = m_sectors[newSec.GetY()][newSec.GetX()];
	newSector.AddUser(pUser);

	SectorAround DeleteSector;
	SectorAround CreateSector;
	SectorPos::CalSectorTransitionMessageTargets(pUser->GetSectorPos(), newSec, DeleteSector, CreateSector);

	// 시야에 사라진 섹터에 있는 캐릭터들에게 내 캐릭터 삭제 메세지 보내기
	CMessage* pDeleteMsg = PacketBuilder::DeleteCharacter(pUser);
	for (int i = 0; i < DeleteSector.m_count; i++)
	{
		SendPacket_SectorOne(BroadCastType::DeleteCharacter, pDeleteMsg, DeleteSector.m_Around[i].GetX(), DeleteSector.m_Around[i].GetY(), pUser);
	}
	CMessage::Free(pDeleteMsg);

	// 새로 시야에 들어온 섹터에 있는 캐릭터들에게 내 캐릭터 생성 메세지 보내기
	CMessage* pCreateMsg = PacketBuilder::CreateOtherCharacter(pUser);
	for (int i = 0; i < CreateSector.m_count; i++)
	{
		SendPacket_SectorOne(BroadCastType::CreateMyCharacterToOther, pCreateMsg, CreateSector.m_Around[i].GetX(), CreateSector.m_Around[i].GetY(), pUser);
	}
	CMessage::Free(pCreateMsg);


	// 시야에 사라진 캐릭터들 및 몬스터에 대한 삭제 메세지를 내 캐릭터에게 보내기
	for (int i = 0; i < DeleteSector.m_count; i++)
	{
		uint16 secX = DeleteSector.m_Around[i].GetX();
		uint16 secY = DeleteSector.m_Around[i].GetY();

		FieldSector& sector = m_sectors[secY][secX];
		for (int count = 0; count < sector.GetUserCount(); count++)
		{
			CMessage* pDeletePlayer = PacketBuilder::DeleteCharacter(sector.GetUser(count));
			SendPacket(pUser->GetSessionID(), pDeletePlayer);
			CMessage::Free(pDeletePlayer);
		}

		for (int count = 0; count < sector.GetMonsterCount(); count++)
		{
			CMessage* pDeleteMonster = PacketBuilder::DeleteMonster(sector.GetMonster(count));
			SendPacket(pUser->GetSessionID(), pDeleteMonster);
			CMessage::Free(pDeleteMonster);

			
		}

		for (int count = 0; count < sector.GetItemCount(); count++)
		{
			CMessage* pDeleteItem = PacketBuilder::DeleteFieldDropItem(sector.GetFieldDropItem(count));
			SendPacket(pUser->GetSessionID(), pDeleteItem);
			CMessage::Free(pDeleteItem);
		}

	}

	// 새롭게 시야에 들어온 캐릭터들 및 몬스터에 대한 생성 메세지를 내 캐릭터에게 보내기
	for (int i = 0; i < CreateSector.m_count; i++)
	{
		uint16 secX = CreateSector.m_Around[i].GetX();
		uint16 secY = CreateSector.m_Around[i].GetY();

		FieldSector& sector = m_sectors[secY][secX];
		for (int count = 0; count < sector.GetUserCount(); count++)
		{
			CMessage* pCreatePlayer = PacketBuilder::CreateOtherCharacter(sector.GetUser(count));
			SendPacket(pUser->GetSessionID(), pCreatePlayer);
			CMessage::Free(pCreatePlayer);
		}

		for (int count = 0; count < sector.GetMonsterCount(); count++)
		{
			CMonster* pMonster = sector.GetMonster(count);

			CMessage* pCreateMonster = PacketBuilder::CreateMonster(pMonster);
			SendPacket(pUser->GetSessionID(), pCreateMonster);
			CMessage::Free(pCreateMonster);

			if (pMonster->GetMonsterState() == EMonsterState::Patrol || pMonster->GetMonsterState() == EMonsterState::Chase || pMonster->GetMonsterState() == EMonsterState::Return)
			{
				CMessage* pMoveMonster = PacketBuilder::MoveMonster(pMonster, pMonster->GetMonsterAITargetLocation());
				SendPacket(pUser->GetSessionID(), pMoveMonster);
				CMessage::Free(pMoveMonster);
			}

		}

		for (int count = 0; count < sector.GetItemCount(); count++)
		{
			FieldDropItem* pItem = sector.GetFieldDropItem(count);

			CMessage* pCreateItem = PacketBuilder::CreateFieldDropItem(pItem);
			SendPacket(pUser->GetSessionID(), pCreateItem);
			CMessage::Free(pCreateItem);
		}

	}

	pUser->SetNewSectorPos(newSec);
}

void FieldGroup::MonsterUpdate()
{
	for (int i = 0; i < MAX_GROSS_FIELD_MONSTER_COUNT; i++)
	{
		// 몬스터 Update시 리젠 성공하면 true 아니면 false
		if (!m_grossMonsterPoolArray[i].MonsterUpdate())
			continue;

		// 리젠 성공시 섹터 처리
		// 섹터에 삽입
		const SectorPos& sectorPos = m_grossMonsterPoolArray[i].GetSectorPos();

		m_sectors[sectorPos.GetY()][sectorPos.GetX()].AddMonster(&m_grossMonsterPoolArray[i]);

		SectorAround Create;
		SectorPos::SectorFind(Create, sectorPos);

		for (int secCount = 0; secCount < Create.m_count; secCount++)
		{
			SendMonsterCreateToSector(&m_grossMonsterPoolArray[i], Create.m_Around[secCount].GetX(), Create.m_Around[secCount].GetY());
		}
	}
}

void FieldGroup::FieldDropItemExpired()
{
	// 전체 자료구조 순회하면서 기간 만료된거 지우기
	FieldDropItem* pItem = nullptr;
	std::unordered_map<uint64, FieldDropItem*>::iterator it = m_dropItemLookUpTable.begin();

	while (it != m_dropItemLookUpTable.end())
	{
		pItem = it->second;

		// 유효 기간 안지났으면 시간만 올리고 pass
		if (pItem->expiredTime < FieldDropItemConst::FIELD_DROP_ITEM_EXPIRED_TIME)
		{
			pItem->expiredTime += FieldConst::UPDATE_LOOP_TIME;
			++it;
			continue;
		}

		// 주변 섹터들에게 아이템 삭제 메세지 보내기
		SendDeleteFieldDropItem(pItem);

		// 섹터에서 아이템 제거
		m_sectors[pItem->sectorPos.GetY()][pItem->sectorPos.GetX()].RemoveItem(pItem);

		// 전체 자료구조에서 제거
		it = m_dropItemLookUpTable.erase(it);

		// 메모리 풀에 반납
		FieldDropItemPool::FreeItem(pItem);
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

					float xpos = MAP_WORLD_OFFSET_X + sx * SECTOR_SIZE + SECTOR_SPAWN_OFFSETX + rand() % (SECTOR_SIZE - SECTOR_SPAWN_OFFSETX - SECTOR_SPAWN_OFFSETX);
					float ypos = MAP_WORLD_OFFSET_Y + sy * SECTOR_SIZE + SECTOR_SPAWN_OFFSETY + rand() % (SECTOR_SIZE - SECTOR_SPAWN_OFFSETY - SECTOR_SPAWN_OFFSETY);

					CMonster& monster = m_grossMonsterPoolArray[monstrSpawnCount++];
					Location loc{ xpos,ypos ,-38775.f };
					monster.Init(m_monsterAllocID, 0, loc, this);
					m_monsterAllocID++;

					m_sectors[sy][sx].AddMonster(&monster);

				}
			}
		}
	}
}

void FieldGroup::CreateFieldDropItem(CMonster& monster)
{
	// 만약 죽었으면 필드 드랍 아이템 생성하기
	if (monster.IsAlive())
		return;

	FieldDropItem* pItem = FieldDropItemPool::CreateItem(monster.GetLocation());
	if (pItem == nullptr)
		return;

	// 섹터 및 관리 자료구조에 넣기
	m_dropItemLookUpTable.insert(std::pair<uint64, FieldDropItem*>(pItem->dropUID, pItem));
	if (!m_sectors[pItem->sectorPos.GetY()][pItem->sectorPos.GetX()].AddItem(pItem))
	{
		// 섹터에 못 넣으면 그냥 다 반납
		m_dropItemLookUpTable.erase(pItem->dropUID);
		FieldDropItemPool::FreeItem(pItem);
		return;
	}

	SendCreateFieldDropItem(pItem);

	return;
}


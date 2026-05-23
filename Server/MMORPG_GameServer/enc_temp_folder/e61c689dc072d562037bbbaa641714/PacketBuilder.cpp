#include <chrono>
#include "ContentsProtocol.h"
#include "ContentsStruct.h"
#include "ContentsDefine.h"
#include "IUser.h"
#include "CUser.h"
#include "CMonster.h"
#include "MemoryPoolTLS.h"
#include "CMessage.h"
#include "PacketBuilder.h"

CMessage* PacketBuilder::CreateMyCharacter(CUser* pUser)
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_CREATE_MY_CHARACTER;
	*pMessage << pUser->GetSessionID();
	*pMessage << pUser->GetX();
	*pMessage << pUser->GetY();
	*pMessage << pUser->GetZ();
	*pMessage << pUser->GetHP();
	*pMessage << pUser->GetMaxHP(timeGetTime());
	*pMessage << pUser->GetMP();
	*pMessage << pUser->GetMaxMP(timeGetTime());
	*pMessage << pUser->GetMPRegenSec();

	return pMessage;
}

CMessage* PacketBuilder::CreateOtherCharacter(CUser* pUser)
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_CREATE_OTHER_CHARACTER;
	*pMessage << pUser->GetSessionID();
	*pMessage << pUser->GetX();
	*pMessage << pUser->GetY();
	*pMessage << pUser->GetZ();
	*pMessage << pUser->GetMoveYaw();
	*pMessage << pUser->GetHP();
	*pMessage << pUser->GetMaxHP(timeGetTime());
	*pMessage << pUser->GetMoveFlag();

	return pMessage;
}

CMessage* PacketBuilder::DeleteCharacter(CUser* pUser)
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_DELETE_CHARACTER;
	*pMessage << pUser->GetSessionID();

	return pMessage;
}

CMessage* PacketBuilder::UpdateCharacterMovement(CUser* pUser)
{
	auto now = std::chrono::system_clock::now();
	uint64 stamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_UPDATE_CHARACTER_MOVEMENT_INPUT;
	*pMessage << pUser->GetSessionID();
	*pMessage << stamp;
	*pMessage << pUser->GetX();
	*pMessage << pUser->GetY();
	*pMessage << pUser->GetZ();
	*pMessage << pUser->GetMoveYaw();
	*pMessage << pUser->GetMoveFlag();

	return pMessage;
}

CMessage* PacketBuilder::SyncMyCharacter(CUser* pUser)
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_SYNC_MY_CHARACTER_POS;
	*pMessage << pUser->GetX();
	*pMessage << pUser->GetY();
	*pMessage << pUser->GetZ();

	return pMessage;
}

CMessage* PacketBuilder::SyncOtherCharacter(CUser* pUser)
{
	auto now = std::chrono::system_clock::now();
	uint64 stamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_SYNC_OTHER_CHARACTER_POS;
	*pMessage << pUser->GetSessionID();
	*pMessage << stamp;
	*pMessage << pUser->GetX();
	*pMessage << pUser->GetY();
	*pMessage << pUser->GetZ();

	return pMessage;
}

CMessage* PacketBuilder::CreateRTTEchoMessage()
{
	auto now = std::chrono::system_clock::now();
	uint64 stamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_RTT_ECHO;
	*pMessage << stamp;

	return pMessage;
}

CMessage* PacketBuilder::AttackLeftSwing(CUser* pUser, float attackYaw)
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_SWING_LEFT_ATTACK;
	*pMessage << pUser->GetSessionID();
	*pMessage << attackYaw;
	*pMessage << pUser->GetLastSwingIndex();

	return pMessage;
}

CMessage* PacketBuilder::StopLeftSwing(CUser* pUser)
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_STOP_LEFT_ATTACK;
	*pMessage << pUser->GetSessionID();

	return pMessage;
}

CMessage* PacketBuilder::HitTarget(uint8 hitPlayerCount, uint8 hitMonsterCount, std::vector<CUser*>& hitPlayerArray, std::vector<CMonster*>& hitMonsterArray)
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_ATTACK_HIT_RESULT;
	*pMessage << hitPlayerCount;
	*pMessage << hitMonsterCount;

	for (int i = 0; i < hitPlayerCount; i++)
	{
		*pMessage << hitPlayerArray[i]->GetSessionID();
		*pMessage << hitPlayerArray[i]->GetHP();
	}

	for (int i = 0; i < hitMonsterCount; i++)
	{
		// todo : 몬스터 넣기
	}

	return pMessage;
}

CMessage* PacketBuilder::UseSkillRes(uint8 skillSlot, bool success)
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_USE_SKILL_RES;
	*pMessage << skillSlot;
	*pMessage << (uint8)(success);

	return pMessage;
}

CMessage* PacketBuilder::UseSkillBroadCast(uint64 id, uint8 skillSlot)
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_USE_SKILL_BROADCAST;
	*pMessage << id;
	*pMessage << skillSlot;

	return pMessage;
}

CMessage* PacketBuilder::CreateMonster(CMonster* pMonster)
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_CREATE_MONSTER;
	*pMessage << pMonster->GetMonsterID();
	*pMessage << pMonster->GetX();
	*pMessage << pMonster->GetY();
	*pMessage << pMonster->GetZ();
	*pMessage << pMonster->GetMoveYaw();
	*pMessage << pMonster->GetMonsterType();
	*pMessage << pMonster->GetHP();
	*pMessage << pMonster->GetMaxHP();

	return pMessage;
}

CMessage* PacketBuilder::DeleteMonster(CMonster* pMonster)
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_DELETE_MONSTER;
	*pMessage << pMonster->GetMonsterID();

	return pMessage;
}

CMessage* PacketBuilder::MoveMonster(CMonster* pMonster, Location& DesLocation)
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_MOVE_MONSTER;
	*pMessage << pMonster->GetMonsterID();
	*pMessage << pMonster->GetX();
	*pMessage << pMonster->GetY();
	*pMessage << pMonster->GetZ();
	*pMessage << pMonster->GetMoveYaw();
	*pMessage << pMonster->GetMoveSpeedPerSec();
	*pMessage << DesLocation.xpos;
	*pMessage << DesLocation.ypos;

	return pMessage;
}

CMessage* PacketBuilder::AttackMonster(CMonster* pMonster, uint64 TargetID, uint16 newHP)
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_HIT_TOPLAYER;
	*pMessage << pMonster->GetMonsterID();
	*pMessage << TargetID;
	*pMessage << pMonster->GetMoveYaw();
	*pMessage << newHP;

	return pMessage;
}

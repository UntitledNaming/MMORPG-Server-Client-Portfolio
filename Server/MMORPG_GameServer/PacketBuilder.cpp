#include <chrono>
#include "ContentsProtocol.h"
#include "ContentsStruct.h"
#include "ContentsDefine.h"
#include "IUser.h"
#include "CUser.h"
#include "CMonster.h"
#include "MemoryPoolTLS.h"
#include "CMessage.h"
#include "FieldSector.h"
#include "CGroup.h"
#include "FieldGroup.h"
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
		*pMessage << hitMonsterArray[i]->GetMonsterID();
		*pMessage << hitMonsterArray[i]->GetHP();
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

CMessage* PacketBuilder::MoveMonster(CMonster* pMonster, const Location& DesLocation)
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_MOVE_MONSTER;
	*pMessage << pMonster->GetMonsterID();
	*pMessage << pMonster->GetX();
	*pMessage << pMonster->GetY();
	*pMessage << pMonster->GetZ();
	*pMessage << pMonster->GetMoveSpeedPerSec();
	*pMessage << DesLocation.xpos;
	*pMessage << DesLocation.ypos;

	FieldGroup::movePacketCount++;

	return pMessage;
}

CMessage* PacketBuilder::StopMonster(CMonster* pMonster, const Location& StopLocation)
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_STOP_MONSTER;
	*pMessage << pMonster->GetMonsterID();
	*pMessage << StopLocation.xpos;
	*pMessage << StopLocation.ypos;
	*pMessage << StopLocation.zpos;


	FieldGroup::stopPacketCount++;

	return pMessage;
}

CMessage* PacketBuilder::AttackMonster(CMonster* pMonster, uint64 TargetID, uint16 newHP)
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_HIT_TOPLAYER;
	*pMessage << pMonster->GetMonsterID();
	*pMessage << TargetID;
	*pMessage << newHP;

	return pMessage;
}

CMessage* PacketBuilder::CreateFieldDropItem(FieldDropItem* pItem)
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_CREATE_FIELD_DROP_ITEM;
	*pMessage << pItem->dropUID;
	*pMessage << static_cast<unsigned long>(pItem->itemID);
	*pMessage << pItem->location.xpos;
	*pMessage << pItem->location.ypos;
	*pMessage << pItem->location.zpos;
	*pMessage << pItem->count;

	return pMessage;
}

CMessage* PacketBuilder::DeleteFieldDropItem(FieldDropItem* pItems)
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_DELETE_FIELD_DROP_ITEM;
	*pMessage << pItems->dropUID;

	return pMessage;
}

CMessage* PacketBuilder::PickUpEquipFieldDropItem(PickUpEquipResult* result)
{
	uint8 randomStatCount = result->randomStatCount;

	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_PICKUP_EQUIPMENT_ITEMS;
	*pMessage << static_cast<unsigned long>(result->itemID);
	*pMessage << result->slotIndex;
	*pMessage << result->count;
	*pMessage << randomStatCount;

	for (int i = 0; i < randomStatCount; i++)
	{
		*pMessage << static_cast<uint8>(result->randomStatResult[i].randomStatType);
		*pMessage << result->randomStatResult[i].randomStatValue;
	}

	return pMessage;
}

CMessage* PacketBuilder::PickUpConsumableFieldDropItem(PickUpConsumableResult* result)
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_PICKUP_CONSUMABLE_ITEMS;
	*pMessage << result->updateSlotCount;
	
	for (int i = 0; i < result->updateSlotCount; i++)
	{
		*pMessage << result->consumableResult[i].slotIndex;
		*pMessage << result->consumableResult[i].newItemCount;
	}

	return pMessage;
}

CMessage* PacketBuilder::DeleteItem(bool Success)
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_DELETE_ITEM;
	*pMessage << Success;

	return pMessage;
}

CMessage* PacketBuilder::UseItem(UseItemResult& result)
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	switch (result.resultType)
	{
	case USE_ITEM_RESULT::CONSUME:
		*pMessage << FieldProtocol::PACKET_SC_USE_CONSUMABLE_ITEM;
		break;

	case USE_ITEM_RESULT::EQUIP:
		*pMessage << FieldProtocol::PACKET_SC_EQUIP_ITEM;
		break;

	case USE_ITEM_RESULT::UNEQUIP:
		*pMessage << FieldProtocol::PACKET_SC_UNEQUIP_ITEM;
		break;
	}

	*pMessage << result.success;
	if (!result.success)
		return pMessage;

	switch (result.resultType)
	{
	case USE_ITEM_RESULT::CONSUME:
		*pMessage << static_cast<uint8>(result.consumableResult.slotType);
		*pMessage << result.consumableResult.newItenCount;
		*pMessage << result.consumableResult.slotIndex;
		break;

	case USE_ITEM_RESULT::EQUIP:
		uint8 updateCount = result.equipResult.updateSlotCount;
		*pMessage << updateCount;
		for (int i = 0; i < updateCount; i++)
		{
			*pMessage << static_cast<uint8>(result.equipResult.resultSlot[i].slotState);
			*pMessage << static_cast<uint8>(result.equipResult.resultSlot[i].slotType);
			*pMessage << result.equipResult.resultSlot[i].slotIndex;
			*pMessage << static_cast<unsigned long>(result.equipResult.resultSlot[i].itemID);
		}
		break;

	case USE_ITEM_RESULT::UNEQUIP:
		*pMessage << result.unEquipResult.inventorySlotIdx;
		break;
	}

	return pMessage;
}

CMessage* PacketBuilder::SwapSlot(bool Success)
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);
	*pMessage << FieldProtocol::PACKET_SC_SWAP_SLOT;
	*pMessage << Success;

	return pMessage;
}

CMessage* PacketBuilder::LevelUp(UserLevelStat& result)
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_LEVEL_UP;
	*pMessage << result.level;
	*pMessage << result.requiredExp;
	*pMessage << result.atk;
	*pMessage << result.def;
	*pMessage << result.maxHP;
	*pMessage << result.maxMP;
	*pMessage << result.hpRegenPerSec;
	*pMessage << result.mpRegenPerSec;

	return pMessage;
}

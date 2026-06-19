#include <chrono>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <array>
#include <set>
#include "ContentsProtocol.h"
#include "ContentsStruct.h"
#include "ContentsDefine.h"
#include "Inventory.h"
#include "Equipment.h"
#include "QuickSlot.h"
#include "CUserItemStorage.h"
#include "IUser.h"
#include "CUser.h"
#include "CMonster.h"
#include "MemoryPoolTLS.h"
#include "CMessage.h"
#include "FieldSector.h"
#include "CGroup.h"
#include "FieldGroup.h"
#include "FieldDropItemPool.h"
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
	*pMessage << pUser->GetMoveYaw();
	*pMessage << pUser->GetHP();
	*pMessage << pUser->GetMP();
	*pMessage << pUser->GetLevel();
	*pMessage << pUser->GetCurrentEXP();

	const Inventory& inven = pUser->GetInventory();
	const Equipment& equip = pUser->GetEquipment();
	const QuickSlot& quick = pUser->GetQuickslot();
	const CUserItemStorage& storage = pUser->GetItemStorage();
	const auto& invenArray = inven.GetInventoryArray();
	const auto& equipArray = equip.GetEquipmentArray();
	const auto& quickArray = quick.GetQuickSlotArray();

	*pMessage << inven.GetUseCount();

	for (int16 i = 0; i < UserInventory::INVENTORY_SLOT_MAX; i++)
	{
		// 슬롯 비어있으면 pass
		if(invenArray[i] == ItemUID::ITEM_UID_INVALID_ID)
			continue;

		*pMessage << i;

		const UserItem* pItem = storage.FindItem(invenArray[i]);
		if (pItem == nullptr)
			continue;

		*pMessage << static_cast<unsigned long>(pItem->itemID);
		*pMessage << pItem->count;

		uint8 randStatCount = pItem->randomStatCount;
		*pMessage << randStatCount;
		for (int i = 0; i < randStatCount; i++)
		{
			*pMessage << static_cast<uint8>(pItem->randomStat[i].randomStatType);
			*pMessage << pItem->randomStat[i].randomStatValue;
		}
	}

	*pMessage << equip.GetUseCount();

	for (uint8 i = (uint8)EQUIP_SLOT::HELMET; i < (uint8)EQUIP_SLOT::MAX; i++)
	{
		if (equipArray[i] == ItemUID::ITEM_UID_INVALID_ID)
			continue;

		*pMessage << i;

		const UserItem* pItem = storage.FindItem(equipArray[i]);
		if (pItem == nullptr)
			continue;

		*pMessage << static_cast<unsigned long>(pItem->itemID);

		uint8 randStatCount = pItem->randomStatCount;
		*pMessage << randStatCount;
		for (int i = 0; i < randStatCount; i++)
		{
			*pMessage << static_cast<uint8>(pItem->randomStat[i].randomStatType);
			*pMessage << pItem->randomStat[i].randomStatValue;
		}
	}

	*pMessage << quick.GetUseCount();
	for (uint8 i = 0; i < UserQuickSlot::QUICK_SLOT_MAX; i++)
	{
		if (quickArray[i] == ItemUID::ITEM_UID_INVALID_ID)
			continue;

		*pMessage << i;

		const UserItem* pItem = storage.FindItem(quickArray[i]);
		if (pItem == nullptr)
			continue;

		*pMessage << static_cast<unsigned long>(pItem->itemID);
		*pMessage << pItem->count;
	}
	

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

CMessage* PacketBuilder::AttackLeftSwing(uint64 sessionID, float attackYaw, uint8 swingIndex)
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_SWING_LEFT_ATTACK;
	*pMessage << sessionID;
	*pMessage << attackYaw;
	*pMessage << swingIndex;

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

CMessage* PacketBuilder::AttackMonster(CMonster* pMonster, uint64 TargetID, int16 newHP)
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
	*pMessage << static_cast<unsigned long>(result->itemID);
	
	*pMessage << result->consumableResult.slotIndex;
	*pMessage << result->consumableResult.newItemCount;

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
	{
		*pMessage << static_cast<uint8>(result.consumableResult.slotType);
		*pMessage << result.consumableResult.newItenCount;
		*pMessage << result.consumableResult.slotIndex;
	}
		break;

	case USE_ITEM_RESULT::EQUIP:
	{
		for (int i = 0; i < 2; i++)
		{
			*pMessage << static_cast<uint8>(result.equipResult.resultSlot[i].slotType);
			*pMessage << result.equipResult.resultSlot[i].slotIndex;
			*pMessage << static_cast<unsigned long>(result.equipResult.resultSlot[i].itemID);
		}
	}
		break;

	case USE_ITEM_RESULT::UNEQUIP:
	{
		*pMessage << result.unEquipResult.inventorySlotIdx;
	}
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

CMessage* PacketBuilder::GainExp(GainEXPResult& result)
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	*pMessage << FieldProtocol::PACKET_SC_GAIN_EXP;
	*pMessage << result.levelUp;
	*pMessage <<result.curEXP;

	if (result.levelUp)
	{
		*pMessage << result.curLevel;
		*pMessage << result.curHP;
		*pMessage << result.curMP;
	}
	return pMessage;
}

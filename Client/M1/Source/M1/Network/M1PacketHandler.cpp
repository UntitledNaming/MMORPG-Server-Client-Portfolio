// Fill out your copyright notice in the Description page of Project Settings.


#include "M1PacketHandler.h"
#include "System\Type\M1Type.h"
#include "System\M1SpawnManager.h"
#include "M1NetworkManager.h"
#include "Controller\M1PlayerController.h"
#include "Ability\M1AbilityTypes.h"
#include "ClientCore/MemoryPoolTLS.h"
#include "ClientCore/CMessage.h"
#include "Kismet/GameplayStatics.h"
#include "ContentsEnum.h"
#include "System\M1ItemManager.h"

M1PacketHandler::M1PacketHandler()
{
}

M1PacketHandler::~M1PacketHandler()
{
}

void M1PacketHandler::Handle_SC_LOGIN_RES(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{

}

void M1PacketHandler::Handle_SC_CHARACTER_SELECT_RES(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{

}

void M1PacketHandler::Handle_SC_CREATE_MY_CHARACTER(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	uint64 charid;
	float xpos;
	float ypos;
	float zpos;
	float yaw;
	uint16 hp;
	uint16 mp;
	uint16 level;
	uint32 curexp;

	*pMessage >> charid;
	*pMessage >> xpos;
	*pMessage >> ypos;
	*pMessage >> zpos;
	*pMessage >> yaw;
	*pMessage >> hp;
	*pMessage >> mp;
	*pMessage >> level;
	*pMessage >> curexp;

	FM1SpawnData Data;
	Data.EntityID = charid;
	Data.CurrentEXP = curexp;
	Data.HP = hp;
	Data.MP = mp;
	Data.MaxHP = hp;
	Data.Location = FVector(xpos, ypos, zpos);
	Data.Rotation = FRotator(0, yaw, 0);
	Data.Level = level;
	Data.MoveFlag = false;

	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();
	UM1ItemManager* ItemManager = NetworkManager->GetItemManager();

	SpawnManager->SpawnMyPlayer(Data);
	ItemManager->ParseAndInitFromSpawn(pMessage);

}

void M1PacketHandler::Handle_SC_CREATE_0THER_CHARACTER(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	uint64 id;
	bool moveflag;
	float xpos;
	float ypos;
	float zpos;
	float yaw;
	uint16 hp;
	uint16 maxhp;

	*pMessage >> id;
	*pMessage >> xpos;
	*pMessage >> ypos;
	*pMessage >> zpos;
	*pMessage >> yaw;
	*pMessage >> hp;
	*pMessage >> maxhp;
	*pMessage >> moveflag;

	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();

	FVector Location(xpos, ypos, zpos);
	FRotator Rotation(0, yaw, 0);

	FM1SpawnData Data;
	Data.EntityID = id;
	Data.Location = Location;
	Data.Rotation = Rotation;
	Data.HP = hp;
	Data.MaxHP = maxhp;
	Data.MoveFlag = moveflag;
	SpawnManager->SpawnOtherPlayer(Data);
}

void M1PacketHandler::Handle_SC_DELETE_CHARACTER(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	uint64 id;

	*pMessage >> id;

	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();

	SpawnManager->DespawnPlayer(id);
}

void M1PacketHandler::Handle_SC_UPDATE_CHARACTER_MOVEMENT_INPUT(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	uint64 id;
	uint64 servertimestamp;
	float  xpos;
	float  ypos;
	float  zpos;
	float  moveyaw;
	bool   moveflag;

	*pMessage >> id;
	*pMessage >> servertimestamp;
	*pMessage >> xpos;
	*pMessage >> ypos;
	*pMessage >> zpos;
	*pMessage >> moveyaw;
	*pMessage >> moveflag;

	FMovementSnapshot Snapshot;
	Snapshot.bMoving = moveflag;
	Snapshot.ServerTimestamp = servertimestamp;
	Snapshot.MoveYaw = moveyaw;
	Snapshot.Position = FVector(xpos, ypos, zpos);

	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();
	SpawnManager->UpdateOtherPlayerMovementInput(id, Snapshot);
}

void M1PacketHandler::Handle_SC_SYNC_MY_CHARACTER_POS(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	float xpos;
	float ypos;
	float zpos;

	*pMessage >> xpos;
	*pMessage >> ypos;
	*pMessage >> zpos;

	FVector location(xpos, ypos, zpos);

	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();
	SpawnManager->SyncMyPlayer(location);
}

void M1PacketHandler::Handle_SC_SYNC_OTHER_CHARACTER_POS(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	uint64 id;
	uint64 servertimestamp;
	float xpos;
	float ypos;
	float zpos;

	*pMessage >> id;
	*pMessage >> servertimestamp;
	*pMessage >> xpos;
	*pMessage >> ypos;
	*pMessage >> zpos;

	FVector location(xpos, ypos, zpos);

	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();
	SpawnManager->SyncOtherPlayer(id, location, servertimestamp);
}

void M1PacketHandler::Handle_SC_RTT_ECHO(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	uint64 time;

	*pMessage >> time;

	NetworkManager->GetSpawnManager()->GetRTTEchoMsg(time);
}

void M1PacketHandler::Handle_SC_SWING_ATTACK(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	uint64 id;
	uint8 swingidx;
	float  facingYaw;

	*pMessage >> id;
	*pMessage >> facingYaw;
	*pMessage >> swingidx;

	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();
	SpawnManager->OnOtherPlayerAttackSwing(id, facingYaw, swingidx);
}

void M1PacketHandler::Handle_SC_STOP_ATTACK(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	uint64 id;

	*pMessage >> id;

	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();
	SpawnManager->OnOtherPlayerAttackStop(id);
}

void M1PacketHandler::Handle_SC_ATTACK_HIT_RESULT(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	uint8 PlayerHitCount;
	uint8 MonsterHitCount;
	*pMessage >> PlayerHitCount;
	*pMessage >> MonsterHitCount;

	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();
	for (uint8 i = 0; i < PlayerHitCount; ++i)
	{
		uint64 id;
		uint16 newHp;
		*pMessage >> id;
		*pMessage >> newHp;
		SpawnManager->ApplyPlayerHitResult(id, (int32)newHp);
	}

	for (uint8 i = 0; i < MonsterHitCount; ++i)
	{
		uint64 id;
		uint16 newHp;
		*pMessage >> id;
		*pMessage >> newHp;
		SpawnManager->ApplyMonsterHitResult(id, (int32)newHp);
	}
}

void M1PacketHandler::Handle_SC_USE_SKILL_RES(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	uint8 SlotID;
	uint8 bSuccess;
	*pMessage >> SlotID >> bSuccess;

	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();
	SpawnManager->OnMyPlayerSkillResponse(static_cast<EAbilitySlot>(SlotID + 1), bSuccess != 0);
}

void M1PacketHandler::Handle_SC_USE_SKILL_BROADCAST(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	uint64 CharacterID;
	uint8  SlotID;
	*pMessage >> CharacterID >> SlotID;

	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();
	SpawnManager->OnOtherCharacterUseSkill(CharacterID, static_cast<EAbilitySlot>(SlotID + 1));
}

void M1PacketHandler::Handle_SC_CREATE_MONSTER(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	uint64 monsterID;

	float xpos;
	float ypos;
	float zpos;
	float yaw;
	uint16 monsterType;
	uint16 hp;
	uint16 maxhp;

	*pMessage >> monsterID;
	*pMessage >> xpos;
	*pMessage >> ypos;
	*pMessage >> zpos;
	*pMessage >> yaw;
	*pMessage >> monsterType;
	*pMessage >> hp;
	*pMessage >> maxhp;

	FVector Location(xpos, ypos, zpos);
	FRotator Rotator(0, yaw, 0);

	FM1SpawnData spawnData;
	spawnData.EntityID = monsterID;
	spawnData.HP = hp;
	spawnData.MaxHP = maxhp;
	spawnData.Location = Location;
	spawnData.Rotation = Rotator;

	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();
	SpawnManager->SpawnMonster(spawnData);
}

void M1PacketHandler::Handle_SC_DELETE_MONSTER(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	uint64 id;

	*pMessage >> id;

	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();

	SpawnManager->DespawnMonster(id);
}

void M1PacketHandler::Handle_SC_MOVE_MONSTER(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	uint64 monsterID;
	float  xpos, ypos, zpos, speed, desxpos, desypos;

	*pMessage >> monsterID;
	*pMessage >> xpos >> ypos >> zpos;
	*pMessage >> speed;
	*pMessage >> desxpos >> desypos;

	FMonsterMove Data;
	Data.MonsterLocation = FVector(xpos, ypos, zpos);
	Data.MoveSpeed       = speed;
	Data.TargetLocation  = FVector(desxpos, desypos, zpos);

	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();
	SpawnManager->OnMonsterMove(monsterID, Data);
}

void M1PacketHandler::Handle_SC_STOP_MONSTER(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	uint64 monsterID;

	float xpos;
	float ypos;
	float zpos;

	*pMessage >> monsterID;
	*pMessage >> xpos;
	*pMessage >> ypos;
	*pMessage >> zpos;


	FVector Location(xpos, ypos, zpos);
	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();
	SpawnManager->OnMonsterStop(monsterID, Location);
}

void M1PacketHandler::Handle_SC_HIT_TOPLAYER(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	uint64 monsterid;
	uint64 targetid;
	uint16 newhp;

	*pMessage >> monsterid;
	*pMessage >> targetid;
	*pMessage >> newhp;

	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();
	SpawnManager->OnMonsterAttack(monsterid, targetid);
	SpawnManager->ApplyPlayerHitResult(targetid, (int32)newhp);
}

void M1PacketHandler::Handle_SC_CHANGE_CHARACTER_MOVEMODE(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	uint64 id;
	uint8  moveMode;

	*pMessage >> id;
	*pMessage >> moveMode;
}

void M1PacketHandler::Handle_SC_CREATE_FIELD_DROP_ITEM(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	uint64 dropID;
	uint32 itemID;
	float  xpos, ypos, zpos;
	uint16 count;

	*pMessage >> dropID;
	*pMessage >> itemID;
	*pMessage >> xpos >> ypos >> zpos;
	*pMessage >> count;

	FCreateDropItem result;
	result.Count = count;
	result.DropID = dropID;
	result.ItemID = itemID;
	result.DropLocation.X = xpos;
	result.DropLocation.Y = ypos;
	result.DropLocation.Z = zpos;

	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();
	SpawnManager->SpawnFieldDropItem(result);
}

void M1PacketHandler::Handle_SC_DELETE_FIELD_DROP_ITEM(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	uint64 dropID;

	*pMessage >> dropID;

	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();
	SpawnManager->DespawnFieldDropItem(dropID);
}

void M1PacketHandler::Handle_SC_PICKUP_EQUIPMENT_ITEMS(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	PickUpEquipResult result = {};

	*pMessage >> result.itemID;
	*pMessage >> result.slotIndex;
	*pMessage >> result.count;
	*pMessage >> result.randomStatCount;

	for (uint8 i = 0; i < result.randomStatCount; ++i)
	{
		uint8 randomStatType = 0;
		*pMessage >> randomStatType;
		*pMessage >> result.randomStatResult[i].randomStatValue;
		result.randomStatResult[i].randomStatType = static_cast<RANDOM_STAT_TYPE>(randomStatType);
	}

	UM1ItemManager* ItemManager = NetworkManager->GetItemManager();
	ItemManager->OnPickupEquipmentItem(result);

}

void M1PacketHandler::Handle_SC_PICKUP_CONSUMABLE_ITEMS(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	PickUpConsumableResult result = {};

	*pMessage >> result.itemID;

	*pMessage >> result.consumableResult.slotIndex;
	*pMessage >> result.consumableResult.newItemCount;

	UM1ItemManager* ItemManager = NetworkManager->GetItemManager();
	ItemManager->OnPickupConsumableItem(result);
}

void M1PacketHandler::Handle_SC_USE_CONSUMABLE_ITEM(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	bool success;
	USE_CONSUMABLE_ITEM_RESULT result = {};

	*pMessage >> success;

	if (success)
	{
		uint8 type;
		*pMessage >> type;
		result.slotType = static_cast<SLOT_TYPE>(type);
		*pMessage >> result.newItenCount;
		*pMessage >> result.slotIndex;
	}


	UM1ItemManager* ItemManager = NetworkManager->GetItemManager();
	ItemManager->OnUseConsumableResult(success,result);
}

void M1PacketHandler::Handle_SC_EQUIP_ITEM(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	bool success;
	EQUIP_ITEM_RESULT result;

	*pMessage >> success;

	if (success)
	{
		for (uint8 i = 0; i < 2; ++i)
		{
			uint8  slotType;
			*pMessage >> slotType;
			*pMessage >> result.resultSlot[i].slotIndex;
			*pMessage >> result.resultSlot[i].itemID;
			result.resultSlot[i].slotType = static_cast<SLOT_TYPE>(slotType);
		}
	}

	UM1ItemManager* ItemManager = NetworkManager->GetItemManager();
	ItemManager->OnEquipItemResult(success, result);
}

void M1PacketHandler::Handle_SC_UNEQUIP_ITEM(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	bool success;
	UNEQUIP_ITEM_RESULT result;

	*pMessage >> success;

	if (success)
	{
		*pMessage >> result.inventorySlotIdx;
	}

	UM1ItemManager* ItemManager = NetworkManager->GetItemManager();
	ItemManager->OnUnequipItemResult(success, result);
}

void M1PacketHandler::Handle_SC_DELETE_ITEM(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	bool success;

	*pMessage >> success;

	UM1ItemManager* ItemManager = NetworkManager->GetItemManager();
	ItemManager->OnDeleteItemResult(success);
}

void M1PacketHandler::Handle_SC_SWAP_SLOT(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	bool success;

	*pMessage >> success;

	UM1ItemManager* ItemManager = NetworkManager->GetItemManager();
	ItemManager->OnSwapSlotResult(success);
}

void M1PacketHandler::Handle_SC_GAIN_EXP(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	bool   levelup;
	uint32 curexp;
	uint16 level;
	uint16 hp;
	uint16 mp;

	*pMessage >> levelup;
	*pMessage >> curexp;

	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();
	if (!levelup)
	{
		SpawnManager->OnGetExp(curexp);
		return;
	}

	*pMessage >> level;
	*pMessage >> hp;
	*pMessage >> mp;

	SpawnManager->OnLevelUp(level, hp, mp, curexp);
}

void M1PacketHandler::Handle_SC_RESPAWN_RES(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	float X, Y, Z, Yaw;
	uint16 HP, MP;

	*pMessage >> X;
	*pMessage >> Y;
	*pMessage >> Z;
	*pMessage >> Yaw;
	*pMessage >> HP;
	*pMessage >> MP;

	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();
	if (!SpawnManager)
		return;

	FVector Location(X, Y, Z);
	SpawnManager->OnRespawn(Location, HP, MP, Yaw);
}
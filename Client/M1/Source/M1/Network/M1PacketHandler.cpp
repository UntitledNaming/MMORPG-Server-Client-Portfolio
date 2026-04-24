// Fill out your copyright notice in the Description page of Project Settings.


#include "M1PacketHandler.h"
#include "System\Type\M1Type.h"
#include "System\M1SpawnManager.h"
#include "M1NetworkManager.h"
#include "ClientCore/MemoryPoolTLS.h"
#include "ClientCore/CMessage.h"

M1PacketHandler::M1PacketHandler()
{
}

M1PacketHandler::~M1PacketHandler()
{
}

void M1PacketHandler::Handle_SC_LOGIN_RES(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{

}

void M1PacketHandler::Handle_SC_CREATE_MY_CHARACTER(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	float xpos;
	float ypos;
	uint16 hp;
	uint16 mp;
	uint16 maxhp;
	uint16 maxmp;

	*pMessage >> xpos;
	*pMessage >> ypos;
	*pMessage >> hp;
	*pMessage >> maxhp;
	*pMessage >> mp;
	*pMessage >> maxmp;

	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();

	FVector Location(xpos, ypos, -38754.0f);
	FRotator Rotation(0, 0, 0);

	FM1SpawnData Data;
	Data.EntityID = -1;
	Data.HP = hp;
	Data.MP = mp;
	Data.ActionType = EM1ActionStateType::None;
	Data.MaxHP = maxhp;
	Data.MaxMP = maxmp;
	Data.Location = Location;
	Data.Rotation = Rotation;

	SpawnManager->SpawnMyPlayer(Data);
}

void M1PacketHandler::Handle_SC_CREATE_0THER_CHARACTER(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	uint64 id;
	float xpos;
	float ypos;
	float yaw;
	uint32 speed;
	uint16 hp;
	uint16 maxhp;
	uint8 action;
	uint8 movemode;

	*pMessage >> id;
	*pMessage >> xpos;
	*pMessage >> ypos;
	*pMessage >> yaw;
	*pMessage >> speed;
	*pMessage >> hp;
	*pMessage >> maxhp;
	*pMessage >> action;
	*pMessage >> movemode;


	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();

	FVector Location(xpos, ypos, -38754.0f);
	FRotator Rotation(0, yaw, 0);

	FM1SpawnData Data;
	Data.EntityID = id;
	Data.HP = hp;
	Data.MoveSpeed = speed;
	Data.MaxHP = maxhp;
	Data.ActionType = static_cast<EM1ActionStateType>(action);
	Data.MoveMode = movemode;

	SpawnManager->SpawnOtehrPlayer(Data);
}

void M1PacketHandler::Handle_SC_DELETE_CHARACTER(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	uint64 id;

	*pMessage >> id;

	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();

	SpawnManager->DespawnPlayer(id);
}

void M1PacketHandler::Handle_SC_UPDATE_CHARACTER_INPUT(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{

}

void M1PacketHandler::Handle_SC_SYNC_MY_CHARACTER_POS(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{

}

void M1PacketHandler::Handle_SC_SYNC_OTHER_CHARACTER_POS(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{

}
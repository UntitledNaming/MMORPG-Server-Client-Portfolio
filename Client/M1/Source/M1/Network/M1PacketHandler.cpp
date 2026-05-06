// Fill out your copyright notice in the Description page of Project Settings.


#include "M1PacketHandler.h"
#include "System\Type\M1Type.h"
#include "System\M1SpawnManager.h"
#include "M1NetworkManager.h"
#include "Controller\M1PlayerController.h"
#include "ClientCore/MemoryPoolTLS.h"
#include "ClientCore/CMessage.h"
#include "Kismet/GameplayStatics.h"

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
	float zpos;
	uint16 hp;
	uint16 mp;
	uint16 maxhp;
	uint16 maxmp;

	*pMessage >> xpos;
	*pMessage >> ypos;
	*pMessage >> zpos;
	*pMessage >> hp;
	*pMessage >> maxhp;
	*pMessage >> mp;
	*pMessage >> maxmp;

	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();

	FVector Location(xpos, ypos, zpos);
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
	bool moveflag;
	float xpos;
	float ypos;
	float zpos;
	float yaw;
	uint16 hp;
	uint16 maxhp;
	uint8 action;
	uint8 movemode;

	*pMessage >> id;
	*pMessage >> xpos;
	*pMessage >> ypos;
	*pMessage >> zpos;
	*pMessage >> yaw;
	*pMessage >> hp;
	*pMessage >> maxhp;
	*pMessage >> action;
	*pMessage >> movemode;
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
	Data.ActionType = static_cast<EM1ActionStateType>(action);
	Data.MoveMode = movemode;
	Data.MoveFlag = moveflag;
	SpawnManager->SpawnOtehrPlayer(Data);
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
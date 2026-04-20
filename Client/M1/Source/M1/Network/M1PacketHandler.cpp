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

	*pMessage >> xpos;
	*pMessage >> ypos;
	*pMessage >> hp;
	*pMessage >> mp;

	AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();

	FVector Location(405411.0f, 397352.0f, -38713.0f);
	FRotator Rotation(0, -100.0f, 0);

	FM1SpawnData Data;
	Data.HP = hp;
	Data.MP = mp;
	Data.ActionType = EM1ActionType::Idle;
	Data.InputMask = InputMask::None;
	Data.MaxHP = 100;
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
	uint16 hp;
	uint8 action;

	*pMessage >> id;
	*pMessage >> xpos;
	*pMessage >> ypos;
	*pMessage >> yaw;
	*pMessage >> hp;
	*pMessage >> action;

}

void M1PacketHandler::Handle_SC_DELETE_CHARACTER(CMessage* pMessage, UM1NetworkManager* NetworkManager)
{
	uint64 id;

	*pMessage >> id;


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
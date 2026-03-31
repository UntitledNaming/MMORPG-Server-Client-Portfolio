// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/M1NetworkManager.h"
#include "ClientCore/NetPacketHeader.h"
#include "ClientCore/CLanGameClient.h"
#include "ClientCore/M1GameClient.h"
#include "ClientCore/CMessage.h"

void UM1NetworkManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CMessage::Init(sizeof(GAMELIB_LANHEADER),0);

	M1Client = new M1GameClient;

	if (ServerIP.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("IP가 비어있습니다! DefaultGame.ini를 확인하세요."));
		return;
	}

	if (!M1Client->Connect(*ServerIP, ServerPort))
	{
		UE_LOG(LogTemp, Error, TEXT("서버와 연결이 되지 않았습니다."));
	}

}

void UM1NetworkManager::Deinitialize()
{
	Super::Deinitialize();
	if (M1Client)
	{
		M1Client->Destroy();
		delete M1Client;
		M1Client = nullptr;
	}

	CMessage::PoolDestroy();
}

void UM1NetworkManager::SendPacket(CMessage* Packet, uint8 RouteType, uint16 ServiceID)
{
	M1Client->SendPacket(Packet, RouteType, ServiceID);
}
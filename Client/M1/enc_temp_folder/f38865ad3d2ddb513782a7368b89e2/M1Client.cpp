// Fill out your copyright notice in the Description page of Project Settings.


#include "M1Client.h"
#include "NetPacketHeader.h"
#include "MemoryPoolTLS.h"
#include "CMessage.h"
#include "ContentsProtocol.h"



M1Client::M1Client()
{
}

M1Client::~M1Client()
{
}

void M1Client::Destroy()
{
	CLanClient::Destroy();
}

void M1Client::OnEnterJoinServer()
{
	CMessage* pMessage = CMessage::Alloc();
	pMessage->Clear(1);

	mpCreateLoginRequest(pMessage);

	SendPacket(pMessage, ERouteType::GROUP, ServiceID::NONE_SERVICE);

	CMessage::Free(pMessage);
}

void M1Client::OnLeaveServer()
{

}

void M1Client::OnRecv(CMessage* pMessage)
{
	pMessage->AddRef();
	PacketQueue.Enqueue(pMessage);
}

void M1Client::OnSend(int sendsize)
{

}

void M1Client::mpCreateLoginRequest(CMessage* pMessage)
{
	*pMessage << AuthProtocol::PACKET_CS_GAME_LOGIN_REQ;
	*pMessage << (uint64)0;

	char temptoken[64] = "123456789";
	pMessage->PutData(temptoken, 64);

}
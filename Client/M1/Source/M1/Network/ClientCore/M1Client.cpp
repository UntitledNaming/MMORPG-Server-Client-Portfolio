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


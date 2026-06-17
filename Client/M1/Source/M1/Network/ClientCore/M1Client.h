// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "CLanGameClient.h"

class M1_API M1Client : public CLanClient
{
public:
	M1Client();
	~M1Client();

	virtual void Destroy() override;

protected:
	virtual void OnEnterJoinServer() override;
	virtual void OnLeaveServer() override;
	virtual void OnRecv(CMessage* pMessage) override;
	virtual void OnSend(int sendsize) override;

public:
	TQueue<CMessage*,EQueueMode::Mpsc> PacketQueue;
};

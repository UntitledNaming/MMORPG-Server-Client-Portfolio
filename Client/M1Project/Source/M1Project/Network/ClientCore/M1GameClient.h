// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CLanGameClient.h"

class CMessage;

class M1PROJECT_API M1GameClient : public CLanClient
{
public:
	M1GameClient();
	~M1GameClient();

	virtual void Destroy() override;

protected:
	virtual void OnEnterJoinServer() override;                                               
	virtual void OnLeaveServer() override;                                                 
	virtual void OnRecv(CMessage* pMessage) override;                                      
	virtual void OnSend(int sendsize) override;                                            
};

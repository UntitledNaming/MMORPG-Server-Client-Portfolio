// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ContentsProtocol.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "M1NetworkManager.generated.h"

class M1Client;
class CMessage;
class UM1NetworkManager;
class AM1SpawnManager;
class UM1ItemManager;

typedef void(*PacketHandlerFunc)(CMessage*, UM1NetworkManager*);

UCLASS(Config = Game)
class M1_API UM1NetworkManager : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UM1NetworkManager, STATGROUP_Tickables); }
    virtual bool IsTickable() const override { return true; }

    AM1SpawnManager* GetSpawnManager();
    UM1ItemManager* GetItemManager();
    void  SetSpawnManager(AM1SpawnManager* Manager);
    void  SendPacket(CMessage* Packet, uint8 RouteType, uint16 ServiceID);
    bool  Disconnect();
    bool  ReConnect();
    bool  ConnectAlive();

private:
    void PacketHandler(CMessage* pMessage);
    void InitFunctorArray();
    void LoadServerConfig();

protected:
    // Config 키워드를 붙이면 .ini 파일의 섹션에서 값을 자동으로 가져옵니다.
    // 지금은 게임서버의 IP, Port지만 실제로는 로그인 서버의 IP, Port로 로그인 서버에 먼저 접속해서
    // 토큰 발급 및 게임 서버의 IP, Port를 가지고 와야 함.

    FString ServerIP;

    int32 ServerPort;

private:
    M1Client* ClientInstance = nullptr;

    PacketHandlerFunc M1FunctorArray[ContentsProtocol::MAX_PACKET_ID];

    AM1SpawnManager* SpawnManager = nullptr;

    UM1ItemManager* ItemManager = nullptr;

    double LastSendTime = 0;
};

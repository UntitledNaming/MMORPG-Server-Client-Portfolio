#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "M1NetworkManager.generated.h"

class M1GameClient;
class CMessage;

UCLASS(Config = Game)
class M1PROJECT_API UM1NetworkManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void  SendPacket(CMessage* Packet, uint8 RouteType, uint16 ServiceID);
    bool  Disconnect();
    bool  ReConnect();                                                                
    bool  ConnectAlive();

protected:
    // Config 키워드를 붙이면 .ini 파일의 섹션에서 값을 자동으로 가져옵니다.
    UPROPERTY(Config)
    FString ServerIP;

    UPROPERTY(Config)
    int32 ServerPort;

private:
    M1GameClient* M1Client = nullptr;
};

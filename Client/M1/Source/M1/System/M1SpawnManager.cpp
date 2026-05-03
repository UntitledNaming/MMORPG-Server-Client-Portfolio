

#include "M1SpawnManager.h"
#include "Network\M1NetworkManager.h"
#include "ContentsDefine.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Controller\M1PlayerController.h"
#include "Character\M1Character.h"
#include "Character\M1BasePlayer.h"
#include "Character\M1LocalPlayer.h"
#include "Character\M1OtherPlayer.h"
#include "Character\M1Monster.h"
#include "Network\ClientCore\CMessage.h"
#include "NetPacketHeader.h"
#include "Kismet/GameplayStatics.h"

AM1SpawnManager::AM1SpawnManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
}

void AM1SpawnManager::BeginPlay()
{
    Super::BeginPlay();

    UGameInstance* GI = GetGameInstance();
    if (GI == nullptr)
        return;


    NetworkManager = GI->GetSubsystem<UM1NetworkManager>();
    if (NetworkManager == nullptr)
        return;

    NetworkManager->SetSpawnManager(this);

    OldRTTCheckTime = FPlatformTime::Seconds();
}

void AM1SpawnManager::Tick(float Deltatime)
{
    Super::Tick(Deltatime);

    double curTime = FPlatformTime::Seconds();
    if (curTime - OldRTTCheckTime >= 1.0f)
    {
        SendRttPacket();
        OldRTTCheckTime = curTime;
    }
}

void AM1SpawnManager::SpawnMyPlayer(FM1SpawnData& Data)
{
    //// 서버로부터 캐릭터 생성 패킷 2번 오거나 코드 실수로 2번 쳤을 때
    //if (PlayerMap.Contains(Data.EntityID))
    //    __debugbreak();

    if (LocalPlayerCharacterClass == nullptr)
        return;


    // 액터 생성은 월드가 있어야 가능하니 월드 얻는데 없으면 그냥 pass
    UWorld* World = GetWorld();
    if (World == nullptr)
        return;


    // 어쨌든 생성하겠다는 의미. 겹치면 위치 조정해서
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AM1LocalPlayer* NewPlayer =
        World->SpawnActor<AM1LocalPlayer>(LocalPlayerCharacterClass, Data.Location, Data.Rotation, Params);

    if (NewPlayer == nullptr)
        return;

    // 플레이어 생성하면 플래그 키고 플레이어 초기화 후 PlayerMap에 넣기
    NewPlayer->ApplySpawnData(Data);
    NewPlayer->SetSpawnFlag(true);
    MyPlayer = NewPlayer;


    // 컨트롤러 가져와서 이 캐릭터에 Possess함. 컨트롤러가 지금 생성한 캐릭터 컨트롤 할 수 있게 함.
    AM1PlayerController* PC = Cast< AM1PlayerController>(UGameplayStatics::GetPlayerController(World, 0));
    if (PC)
    {
        PC->Possess(NewPlayer);
        PC->SetCachedPlayer(NewPlayer);
        PC->SetControlRotation(FRotator(0.f, 0.f, 0.f));
        NewPlayer->SetActorRotation(FRotator(0.f, 0.f, 0.f));
        
        SendRttPacket();
    }

}

void AM1SpawnManager::SpawnOtehrPlayer(FM1SpawnData& Data)
{
    if (PlayerMap.Contains(Data.EntityID))
        return;

    if (OtherPlayerCharacterClass == nullptr)
        return;

    UWorld* World = GetWorld();
    if (World == nullptr)
        return;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AM1OtherPlayer* NewCharacter =
        World->SpawnActor<AM1OtherPlayer>(
            OtherPlayerCharacterClass,
            Data.Location,
            Data.Rotation,
            Params);

    if (NewCharacter == nullptr)
        return;

    NewCharacter->ApplySpawnData(Data);
    NewCharacter->SetSpawnFlag(true);
    PlayerMap.Add(Data.EntityID, NewCharacter);
}

void AM1SpawnManager::SpawnMonster(FM1SpawnData& Data)
{

}

void AM1SpawnManager::DespawnPlayer(uint64 EntityID)
{
    AM1OtherPlayer** Found = PlayerMap.Find(EntityID);
	if (Found == nullptr || *Found == nullptr)
		return;

	(*Found)->Destroy();
	PlayerMap.Remove(EntityID);
}

void AM1SpawnManager::DespawnMonster(uint64 EntityID)
{
    AM1Monster** Found = MonsterMap.Find(EntityID);
    if (Found == nullptr || *Found == nullptr)
        return;

    (*Found)->Destroy();
    MonsterMap.Remove(EntityID);
}

void AM1SpawnManager::SyncMyPlayer(FVector& Location)
{
    if (LocalPlayerCharacterClass == nullptr || MyPlayer == nullptr)
        return;

    MyPlayer->SetActorLocation(Location);
}

void AM1SpawnManager::SyncOtherPlayer(uint64 EntityID, FVector& Location, uint64 ServerTimestamp)
{
    AM1OtherPlayer** Found = PlayerMap.Find(EntityID);
    if (Found == nullptr || *Found == nullptr)
        return;

    (*Found)->OnReceiveSyncPacket(ServerTimestamp, Location);
}

void AM1SpawnManager::UpdateOtherPlayerMovementInput(uint64 EntityID, FMovementSnapshot& Snapshot)
{
    AM1OtherPlayer** Found = PlayerMap.Find(EntityID);
    if (Found == nullptr || *Found == nullptr)
        return;

    (*Found)->OnReceiveMovementPacket(Snapshot);
}

AM1Character* AM1SpawnManager::FindPlayer(uint64 EntityID) const
{
    AM1OtherPlayer* const* Found = PlayerMap.Find(EntityID);

	if (Found == nullptr)
		return nullptr;

	return *Found;
}

AM1Character* AM1SpawnManager::FindMonster(uint64 EntityID) const
{
    AM1Monster* const* Found = MonsterMap.Find(EntityID);

    if (Found == nullptr)
        return nullptr;

    return *Found;
}

void AM1SpawnManager::SendRttPacket()
{
    if (NetworkManager == nullptr)
        return;

    CMessage* pMessage = CMessage::Alloc();
    pMessage->Clear(1);

    LastSendTime = FPlatformTime::Seconds();
    mpCreateRttPacket(pMessage, LastSendTime);

    NetworkManager->SendPacket(pMessage, static_cast<uint8>(ERouteType::GROUP), ServiceID::NONE_SERVICE);

    CMessage::Free(pMessage);
}

void AM1SpawnManager::mpCreateRttPacket(CMessage* pMessage, double Time)
{
    *pMessage << FieldProtocol::PACKET_CS_RTT_SEND;
    *pMessage << Time;
}

void AM1SpawnManager::GetRTTEchoMsg()
{
    RTT = FPlatformTime::Seconds() - LastSendTime;
    RTTRecv = true;
    GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green,
        FString::Printf(TEXT("RTT : %f"), RTT * 1000));

}
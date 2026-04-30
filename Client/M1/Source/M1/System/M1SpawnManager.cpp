

#include "M1SpawnManager.h"
#include "Network\M1NetworkManager.h"
#include "ContentsDefine.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Controller\M1PlayerController.h"
#include "Character\M1Character.h"
#include "Character\M1Player.h"
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

    if (FPlatformTime::Seconds() - OldRTTCheckTime >= 1.0f)
    {
        SendRttPacket();
        OldRTTCheckTime += 1;
    }
}

void AM1SpawnManager::SpawnMyPlayer(FM1SpawnData& Data)
{
    //// 서버로부터 캐릭터 생성 패킷 2번 오거나 코드 실수로 2번 쳤을 때
    //if (PlayerMap.Contains(Data.EntityID))
    //    __debugbreak();

    if (PlayerCharacterClass == nullptr)
        return;


    // 액터 생성은 월드가 있어야 가능하니 월드 얻는데 없으면 그냥 pass
    UWorld* World = GetWorld();
    if (World == nullptr)
        return;


    // 어쨌든 생성하겠다는 의미. 겹치면 위치 조정해서
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AM1Player* NewPlayer =
        World->SpawnActor<AM1Player>(PlayerCharacterClass, Data.Location, Data.Rotation, Params);

    if (NewPlayer == nullptr)
        return;

    // 플레이어 생성하면 플래그 키고 플레이어 초기화 후 PlayerMap에 넣기
    NewPlayer->bIsMyPlayer = true;
    NewPlayer->ApplySpawnData(Data);

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

    if (PlayerCharacterClass == nullptr)
        return;

    UWorld* World = GetWorld();
    if (World == nullptr)
        return;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AM1Player* NewCharacter =
        World->SpawnActor<AM1Player>(
            PlayerCharacterClass,
            Data.Location,
            Data.Rotation,
            Params);

    if (NewCharacter == nullptr)
        return;

    NewCharacter->bIsMyPlayer = false;
    NewCharacter->ApplySpawnData(Data);
    PlayerMap.Add(Data.EntityID, NewCharacter);
}

void AM1SpawnManager::SpawnMonster(FM1SpawnData& Data)
{

}

void AM1SpawnManager::DespawnPlayer(uint64 EntityID)
{
    AM1Player** Found = PlayerMap.Find(EntityID);
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
    if (PlayerCharacterClass == nullptr || MyPlayer == nullptr)
        return;

    MyPlayer->SetActorLocation(Location);
}

void AM1SpawnManager::SyncOtherPlayer(uint64 EntityID, FVector& Location)
{
    AM1Player** Found = PlayerMap.Find(EntityID);
    if (Found == nullptr || *Found == nullptr)
        return;

    (*Found)->SetActorLocation(Location);
}

void AM1SpawnManager::UpdateOtherPlayerMovementInput(uint64 EntityID, FVector& Location, FRotator& Rotation, bool Moveflag)
{
    AM1Player** Found = PlayerMap.Find(EntityID);
    if (Found == nullptr || *Found == nullptr)
        return;


    AM1Player* OtherPlayer = *Found;


    const bool  bOldMoveFlag = OtherPlayer->GetServerMoveFlag();
    const float Dist = FVector::Dist2D(OtherPlayer->GetActorLocation(), Location);

    // 현재 해당 캐릭터는 정지한 상태인데 이동 시작 패킷이 온것이면
    // 이동 플래그만 켜주고 리턴
    // Tick 함수에서 해당 방향으로 이동할 것임.
    if (bOldMoveFlag == false && Moveflag == true)
    {
        OtherPlayer->SetServerMoveFlag(Moveflag);

        // 서버로 부터 받은거는 정지 플래그인데 현재 수렴중이면 
        if (OtherPlayer->GetStopCorrectionFlag() == true)
        {
            OtherPlayer->SetStopCorrectionFlag(false);
        }

    }

    // 해당 캐릭터가 이동중인데 정지 패킷이 온 경우
    else if (bOldMoveFlag == true && Moveflag == false)
    {
        OtherPlayer->SetServerMoveFlag(Moveflag);
        OtherPlayer->SetStopCorrectionFlag(true);
        OtherPlayer->SetStopTargetLocation(Location);
    }


    // 현재 캐릭터 위치랑 패킷으로 받은 위치 차이가 크면 바로 이동
    if (Dist >= ClientMovement::REMOTE_PLAYER_POS_SNAP_DIST_CM)
    {
        OtherPlayer->SetActorLocation(Location);
    }

    // 이동 방향은 바로 변경
    OtherPlayer->SetActorRotation(Rotation);
    return;
}

AM1Character* AM1SpawnManager::FindPlayer(uint64 EntityID) const
{
    AM1Player* const* Found = PlayerMap.Find(EntityID);

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
    double RecvTime = FPlatformTime::Seconds();

    UE_LOG(LogTemp, Warning, TEXT("RTT = %.3f ms"), (RecvTime - LastSendTime) * 1000.0);
}
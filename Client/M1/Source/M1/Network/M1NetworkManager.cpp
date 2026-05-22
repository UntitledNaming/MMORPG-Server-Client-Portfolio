#include "M1NetworkManager.h"
#include "NetPacketHeader.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "ClientCore/CLanGameClient.h"
#include "ClientCore/M1Client.h"
#include "ClientCore/MemoryPoolTLS.h"
#include "ClientCore/CMessage.h"
#include "System\M1SpawnManager.h"
#include "M1PacketHandler.h"
#include "EngineUtils.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "GameFramework/GameUserSettings.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

void UM1NetworkManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadServerConfig();

	//DisableBackgroundIdle();

	// 배열 초기화
	InitFunctorArray();

	CMessage::Init(sizeof(GAMELIB_LANHEADER),0);

	ClientInstance = new M1Client;

	if (ServerIP.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("IP가 비어있습니다! DefaultGame.ini를 확인하세요."));
		return;
	}

	if (!ClientInstance->Connect(*ServerIP, ServerPort))
	{
		UE_LOG(LogTemp, Error, TEXT("서버와 연결이 되지 않았습니다."));
	    return;
	}

	// 백그라운드 클라 느려지는 것 방지
	if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("t.IdleWhenNotForeground")))
	{
		CVar->Set(0, ECVF_SetByCode);
	}

	// FPS 제한
	if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("t.MaxFPS")))
	{
		CVar->Set(60.0f, ECVF_SetByCode);
	}

	// 해상도 / 창모드 / VSync 설정
	if (GEngine)
	{
		if (UGameUserSettings* Settings = GEngine->GetGameUserSettings())
		{
			Settings->SetFullscreenMode(EWindowMode::Windowed);
			Settings->SetScreenResolution(FIntPoint(960, 540));
			Settings->SetVSyncEnabled(false);
			Settings->SetFrameRateLimit(60.0f);

			Settings->ApplySettings(false);
			Settings->SaveSettings();
		}
	}
}

void UM1NetworkManager::Deinitialize()
{
	Super::Deinitialize();

	if (ClientInstance)
	{
		ClientInstance->Disconnect();
		ClientInstance->Destroy();
		delete ClientInstance;
		ClientInstance = nullptr;
	}

	//CMessage::PoolDestroy();
}

void UM1NetworkManager::Tick(float DeltaTime)
{
	if (ClientInstance == nullptr)
		return;

	if (SpawnManager == nullptr)
		return;

	if (!ClientInstance->ConnectAlive())
	{
		UWorld* World = GetGameInstance()->GetWorld();

		if (!World)
			return;

		APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);

		UKismetSystemLibrary::QuitGame(
			World,
			PC,
			EQuitPreference::Quit,
			false
		);
	}


	CMessage* pMessage = nullptr;

	while (ClientInstance->PacketQueue.Dequeue(pMessage))
	{
		PacketHandler(pMessage);
		CMessage::Free(pMessage);
	}

	// todo : 연결 끊겼으면 재연결
}

AM1SpawnManager* UM1NetworkManager::GetSpawnManager()
{
	return SpawnManager;
}

void  UM1NetworkManager::SetSpawnManager(AM1SpawnManager* Manager)
{
	SpawnManager = Manager;
}

void UM1NetworkManager::SendPacket(CMessage* Packet, uint8 RouteType, uint16 ServiceID)
{
	ClientInstance->SendPacket(Packet, (ERouteType)RouteType, ServiceID);
}

bool  UM1NetworkManager::Disconnect()
{
	return ClientInstance->Disconnect();
}

bool  UM1NetworkManager::ReConnect()
{
	return ClientInstance->ReConnect();
}

bool  UM1NetworkManager::ConnectAlive()
{
	return ClientInstance->ConnectAlive();
}

void UM1NetworkManager::PacketHandler(CMessage* pMessage)
{
	uint16 type;
	*pMessage >> type;

	if (type < ContentsProtocol::MAX_PACKET_ID && M1FunctorArray[type] != nullptr)
		M1FunctorArray[type](pMessage, this);
	else
		UE_LOG(LogTemp, Warning, TEXT("Unknown Packet ID: %d"), type);

	CMessage::Free(pMessage);
}

void UM1NetworkManager::InitFunctorArray()
{
	for (int i = 0; i < ContentsProtocol::MAX_PACKET_ID; ++i)
	{
		M1FunctorArray[i] = nullptr;
	}

	// 구현된 함수 포인터 배열에 삽입
	M1FunctorArray[FieldProtocol::PACKET_SC_CREATE_MY_CHARACTER] = &M1PacketHandler::Handle_SC_CREATE_MY_CHARACTER;
	M1FunctorArray[FieldProtocol::PACKET_SC_CREATE_OTHER_CHARACTER] = &M1PacketHandler::Handle_SC_CREATE_0THER_CHARACTER;
	M1FunctorArray[FieldProtocol::PACKET_SC_DELETE_CHARACTER] = &M1PacketHandler::Handle_SC_DELETE_CHARACTER;
	M1FunctorArray[FieldProtocol::PACKET_SC_UPDATE_CHARACTER_MOVEMENT_INPUT] = &M1PacketHandler::Handle_SC_UPDATE_CHARACTER_MOVEMENT_INPUT;
	M1FunctorArray[FieldProtocol::PACKET_SC_SYNC_MY_CHARACTER_POS] = &M1PacketHandler::Handle_SC_SYNC_MY_CHARACTER_POS;
	M1FunctorArray[FieldProtocol::PACKET_SC_SYNC_OTHER_CHARACTER_POS] = &M1PacketHandler::Handle_SC_SYNC_OTHER_CHARACTER_POS;
	M1FunctorArray[FieldProtocol::PACKET_SC_RTT_ECHO] = &M1PacketHandler::Handle_SC_RTT_ECHO;
	M1FunctorArray[FieldProtocol::PACKET_SC_SWING_LEFT_ATTACK]        = &M1PacketHandler::Handle_SC_SWING_ATTACK;
	M1FunctorArray[FieldProtocol::PACKET_SC_STOP_LEFT_ATTACK]         = &M1PacketHandler::Handle_SC_STOP_ATTACK;
	M1FunctorArray[FieldProtocol::PACKET_SC_ATTACK_HIT_RESULT]        = &M1PacketHandler::Handle_SC_ATTACK_HIT_RESULT;
	M1FunctorArray[FieldProtocol::PACKET_SC_USE_SKILL_RES]            = &M1PacketHandler::Handle_SC_USE_SKILL_RES;
	M1FunctorArray[FieldProtocol::PACKET_SC_USE_SKILL_BROADCAST]      = &M1PacketHandler::Handle_SC_USE_SKILL_BROADCAST;
	M1FunctorArray[FieldProtocol::PACKET_SC_CREATE_MONSTER]           = &M1PacketHandler::Handle_SC_CREATE_MONSTER;
	M1FunctorArray[FieldProtocol::PACKET_SC_DELETE_MONSTER]           = &M1PacketHandler::Handle_SC_DELETE_MONSTER;
	M1FunctorArray[FieldProtocol::PACKET_SC_MOVE_MONSTER]           = &M1PacketHandler::Handle_SC_MOVE_MONSTER;
	M1FunctorArray[FieldProtocol::PACKET_SC_HIT_TOPLAYER]           = &M1PacketHandler::Handle_SC_HIT_TOPLAYER;
}

void UM1NetworkManager::LoadServerConfig()
{
	if (GConfig == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("GConfig is null"));
		return;
	}

	FString ConfigPath;

#if WITH_EDITOR
	// 에디터: .uproject 있는 프로젝트 루트
	ConfigPath = FPaths::Combine(
		FPaths::ProjectDir(),
		TEXT("ServerConfig.ini")
	);
#else
	// 패키징 후: exe 실행 위치
	ConfigPath = FPaths::Combine(
		FPaths::LaunchDir(),
		TEXT("ServerConfig.ini")
	);
#endif

	bool bReadIP = GConfig->GetString(
		TEXT("Network"),
		TEXT("ServerIP"),
		ServerIP,
		ConfigPath
	);

	bool bReadPort = GConfig->GetInt(
		TEXT("Network"),
		TEXT("ServerPort"),
		ServerPort,
		ConfigPath
	);

	if (!bReadIP || !bReadPort)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to read ServerConfig.ini: %s"), *ConfigPath);
		return;
	}
}


#include "Character/M1BasePlayer.h"

AM1BasePlayer::AM1BasePlayer()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AM1BasePlayer::BeginPlay()
{
	Super::BeginPlay();

	// 초기값 브로드캐스트
	OnManaChanged.Broadcast(MP, MaxMP);
	OnExpChanged.Broadcast(CurrentExp, RequiredExp);
}

void AM1BasePlayer::ApplySpawnData(const FM1SpawnData& Data)
{
	Super::ApplySpawnData(Data);

	MP = Data.MP;
	MaxMP = Data.MaxMP;
	MoveMode = Data.MoveMode;
	CurrentExp = Data.CurrentEXP;
	RequiredExp = Data.RequiredEXP;
	Level = Data.Level;
	ActionType = static_cast<uint8>(Data.ActionType);
	bMoving = Data.MoveFlag;
}


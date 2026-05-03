#include "Character/M1BasePlayer.h"

void AM1BasePlayer::ApplySpawnData(const FM1SpawnData& Data)
{
	Super::ApplySpawnData(Data);

	MP = Data.MP;
	MaxMP = Data.MaxMP;
	MoveMode = Data.MoveMode;
	ActionType = static_cast<uint8>(Data.ActionType);
}
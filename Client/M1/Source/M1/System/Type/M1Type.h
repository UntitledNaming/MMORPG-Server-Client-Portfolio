#pragma once

#include "CoreMinimal.h"
#include "ContentsEnum.h"

//////////////////////////////////////////
// 0 : Stop  / 1 : Walk / 2 : Run
/////////////////////////////////////////

namespace Client_InputMask
{
    constexpr uint16 None = 1 << 0;
    constexpr uint16 North = 1 << 1;
    constexpr uint16 South = 1 << 2;
    constexpr uint16 East = 1 << 3;
    constexpr uint16 West = 1 << 4;
}

struct FM1SpawnData
{
    uint64   EntityID = 0; 

    FVector  Location = FVector::ZeroVector;

    FRotator Rotation = FRotator::ZeroRotator;

    uint8    MoveMode = 0; 

    uint32   MoveSpeed = 0;

    int32    HP = 0;

    int32    MaxHP = 0;

    int32    MP = 0;

    int32    MaxMP = 0;

    EM1ActionStateType ActionType = EM1ActionStateType::None;
};
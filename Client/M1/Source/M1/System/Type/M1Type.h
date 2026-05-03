#pragma once

#include "CoreMinimal.h"
#include "ContentsEnum.h"

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

    int32    HP = 0;

    int32    MaxHP = 0;

    int32    MP = 0;

    int32    MaxMP = 0;

    EM1ActionStateType ActionType = EM1ActionStateType::None;

    bool MoveFlag = false;
};


struct FMovementSnapshot
{
    uint64   ServerTimestamp = 0;
    FVector  Position = FVector::ZeroVector;
    float    MoveYaw = 0.f;
    bool     bMoving = false;
};


template<typename T, int32 Size>
struct TCircularSnapBuffer
{
    T     Data[Size];
    int32 Head = 0;
    int32 Count = 0;

    void Add(const T& Item)
    {
        Data[Head] = Item;
        Head = (Head + 1) % Size;
        if (Count < Size) Count++;
    }

    void Reset()
    {
        Head = 0;
        Count = 0;
    }

    // 오래된 순서부터 접근
    T& operator[](int32 Idx)
    {
        int32 Start = (Head - Count + Size) % Size;
        return Data[(Start + Idx) % Size];
    }

    T& Last() { return (*this)[Count - 1]; }
};
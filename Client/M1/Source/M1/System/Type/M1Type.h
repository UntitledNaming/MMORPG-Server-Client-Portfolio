#pragma once

#include "CoreMinimal.h"
#include "ContentsEnum.h"

struct FM1SpawnData
{
    uint64   EntityID = 0; 

    FVector  Location = FVector::ZeroVector;

    FRotator Rotation = FRotator::ZeroRotator;

    uint16    HP = 0;

    uint16    MaxHP = 0;

    uint16    MP = 0;

    uint16    MaxMP = 0;

    uint16    Level = 1;

    uint16   MPRegenPerSec = 0;

    float    CurrentEXP = 0.f;

    float    RequiredEXP = 100.f;

    bool     MoveFlag = false;
};


struct FMovementSnapshot
{
    uint64   ServerTimestamp = 0;
    FVector  Position = FVector::ZeroVector;
    float    MoveYaw = 0.f;
    bool     bMoving = false;
};

struct FMonsterMove
{
    FVector MonsterLocation;
    FVector TargetLocation;
    float   MoveSpeed;
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
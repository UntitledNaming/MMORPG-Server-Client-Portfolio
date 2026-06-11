#pragma once

#include "CoreMinimal.h"
#include "ContentsEnum.h"

struct FM1SpawnData
{
    uint64   EntityID = 0; 

    FVector  Location = FVector::ZeroVector;

    FRotator Rotation = FRotator::ZeroRotator;

    int16    HP = 0;

    int16    MaxHP = 0;

    int16    MP = 0;

    int16    MaxMP = 0;

    uint16   Level = 1;

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
    FVector       MonsterLocation;
    FVector       TargetLocation;
    float         MoveSpeed;
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

struct FRandomStat
{
    RANDOM_STAT_TYPE  RandomStatType;
    uint16            RandomStatValue;
};


struct FItemSlotData
{
    ITEM_ID             ItemID;      // 0 == Empty
    uint16              Count;       
    TArray<FRandomStat> RandomStats; // Only Equipment Used
};

UENUM()
enum class ESlotRequestType : uint8
{
    None,
    Use,
    Swap,
    Delete,
};

struct FPendingSlotRequest
{
    ESlotRequestType RequestType = ESlotRequestType::None;

    SLOT_TYPE FromSlotType;
    int16 FromSlotIndex;

    // Swap 전용
    SLOT_TYPE ToSlotType;
    int16 ToSlotIndex;
};

struct FCreateDropItem
{
    uint64  DropID = 0;
    ITEM_ID ItemID = 0;
    FVector DropLocation = FVector::ZeroVector;
    uint16  Count = 0;
};
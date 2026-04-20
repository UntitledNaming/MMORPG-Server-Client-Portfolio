// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "M1Type.generated.h"


UENUM(BlueprintType)
enum class EM1ActionType : uint8
{
    Idle,
    Move,
    Attack,
    Skill,
    Dead,
    Chase,
};

namespace InputMask
{
    constexpr uint16 None = 1 << 0;
    constexpr uint16 North = 1 << 1;
    constexpr uint16 South = 1 << 2;
    constexpr uint16 East = 1 << 3;
    constexpr uint16 West = 1 << 4;
}

USTRUCT(BlueprintType)
struct FM1SpawnData
{
    GENERATED_BODY()

    UPROPERTY()
    uint64 EntityID = 0; 

    UPROPERTY()
    FVector Location = FVector::ZeroVector;

    UPROPERTY()
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY()
    EM1ActionType ActionType = EM1ActionType::Idle;

    UPROPERTY()
    uint16 InputMask = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 HP = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 MaxHP = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 MP = 0;
};
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "M1/System/Type/M1Type.h"
#include "M1Character.generated.h"

struct FInputActionValue;

UCLASS()
class M1_API AM1Character : public ACharacter
{
	GENERATED_BODY()

public:
	AM1Character();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

public:
    FORCEINLINE bool IsDeadState()
    {
        if (HP == 0)
            return true;

       return false;
    }

    FORCEINLINE uint8 GetActionType() const
    {
        return ActionType;
    }

    FORCEINLINE bool GetIsSpawnInit() const
    {
        return bIsSpawnInit;
    }

    FORCEINLINE bool GetServerMoveFlag() const
    {
        return bServerMoveFlag;
    }

    FORCEINLINE bool GetRenderMove() const
    {
        return bRenderMoveFlag;
    }

    FORCEINLINE bool GetStopCorrectionFlag() const
    {
        return bNeedStopCorrection;
    }

    FORCEINLINE void SetServerMoveFlag(bool isMoving)
    {
        bServerMoveFlag = isMoving;
    }

    FORCEINLINE uint32 GetMoveSpeed() const
    {
        return MoveSpeed;
    }

    FORCEINLINE void SetMoveSpeed(uint32 InMoveSpeed)
    {
        MoveSpeed = InMoveSpeed;
    }

    FORCEINLINE void SetStopCorrectionFlag(bool StopCorrection)
    {
        bNeedStopCorrection = StopCorrection;
    }

    FORCEINLINE void SetStopTargetLocation(FVector& StopLocation)
    {
        StopTargetLocation = StopLocation;
    }

public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TObjectPtr<class UWidgetComponent> HpBarComponent;

    UPROPERTY(VisibleAnywhere)
    bool bIsMyPlayer = false;

    UPROPERTY(VisibleAnywhere)
    bool bIsSpawnInit = false;      


protected:
    /////////////////////////////////////////////////////////////////////////
    // 프로토콜을 통해 얻는 값
    /////////////////////////////////////////////////////////////////////////

    UPROPERTY(VisibleAnywhere)
    uint64 EntityID = 0;

    UPROPERTY(VisibleAnywhere)
    uint8 ActionType = 0;

    UPROPERTY(VisibleAnywhere)
    int32 HP = 0;

    UPROPERTY(VisibleAnywhere)
    int32 MaxHP = 0;

    UPROPERTY(VisibleAnywhere)
    uint32 MoveSpeed = 0;                             // 렌더링 할 때 기준값이 되는 속도값

    UPROPERTY(VisibleAnywhere)
    bool bServerMoveFlag = false;                     // 서버가 보낸 해당 캐릭터 현재 이동 상태

    UPROPERTY(VisibleAnywhere)
    bool bRenderMoveFlag = false;                     // 이 클라에서 해당 캐릭터에 대한 움직임이 보여야 하는지에 대한 상태
     
    UPROPERTY(VisibleAnywhere)
    bool bNeedStopCorrection = false;                 // 정지 좌표까지 수렴 중인지에 대한 플래그

    UPROPERTY(VisibleAnywhere)
    FVector StopTargetLocation = FVector::Zero();

public:
    virtual void ApplySpawnData(const FM1SpawnData& Data);
};

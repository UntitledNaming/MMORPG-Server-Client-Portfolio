// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "M1Character.h"
#include "M1Player.generated.h"

UCLASS()
class M1_API AM1Player : public AM1Character
{
	GENERATED_BODY()

public:
    AM1Player();

    FORCEINLINE uint8 GetMoveMode() const
    {
        return MoveMode;
    }


    FORCEINLINE void StartAutoLeftAttack()
    {
        ActionType = static_cast<uint8>(EM1ActionStateType::Attack);
        bAutoAttack = true;
    }

    FORCEINLINE void StopAutoLeftAttack()
    {
        if (ActionType == static_cast<uint8>(EM1ActionStateType::Attack))
        {
            ActionType = static_cast<uint8>(EM1ActionStateType::None);
        }
        bAutoAttack = false;
    }

    FORCEINLINE bool GetJumpRequest()
    {
        return bJumpRequest;
    }

    FORCEINLINE void SetJumpRequest(bool Flag)
    {
        bJumpRequest = Flag;
    }


protected:
    virtual void BeginPlay() override;

public:
    virtual void ApplySpawnData(const FM1SpawnData& Data);

private:
    void UpdateAutoAttack();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    TObjectPtr<class USpringArmComponent> SpringArmComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    TObjectPtr<class UCameraComponent> CameraComponent;
	
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
    FString NickName;

    UPROPERTY(VisibleAnywhere)
    int32 MP = 0;

    UPROPERTY(VisibleAnywhere)
    int32 MaxMP = 0;

    UPROPERTY(VisibleAnywhere)
    uint8 MoveMode = 0;

    UPROPERTY(VisibleAnywhere)
    bool bAutoAttack = false;

    UPROPERTY(VisibleAnywhere)
    bool bJumpRequest = false;
};

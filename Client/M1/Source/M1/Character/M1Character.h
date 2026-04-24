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

public:	
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

private:
    void OnMove(const FInputActionValue& Value);

    void OnLook(const FInputActionValue& Value);

    void OnJumpStart();

    void OnJumpEnd();

    void OnAttackStart();

    void OnAttackEnd();

    void DoMove(float Right, float Forward);

    void DoLook(float Yaw, float Pitch);

public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TObjectPtr<class UWidgetComponent> HpBarComponent;

    UPROPERTY(VisibleAnywhere)
    uint64 EntityID = 0;

    UPROPERTY(VisibleAnywhere)
    bool bIsMyPlayer = false;

    UPROPERTY(VisibleAnywhere)
    bool bIsSpawnInit = false;      // 하위 클래스에서 true로 변경 필요

    UPROPERTY(VisibleAnywhere)
    uint8 ActionType = 0;

    UPROPERTY(VisibleAnywhere)
    int32 HP = 0;

    UPROPERTY(VisibleAnywhere)
    int32 MaxHP = 0;

    UPROPERTY(VisibleAnywhere)
    uint32 MoveSpeed = 0;

public:
    virtual void Destroy();
    virtual void ApplySpawnData(const FM1SpawnData& Data);
};

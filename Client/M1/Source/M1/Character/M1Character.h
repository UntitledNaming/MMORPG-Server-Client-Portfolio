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
    EM1ActionType ActionType = EM1ActionType::Idle;

    UPROPERTY(VisibleAnywhere)
    uint16 InputMask = 0;

    UPROPERTY(VisibleAnywhere)
    int32 HP = 0;

    UPROPERTY(VisibleAnywhere)
    int32 MaxHP = 0;

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<class UInputMappingContext> TestIMC = nullptr;

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<class UInputAction> TestJumpAction = nullptr;

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<class UInputAction> TestMoveAction = nullptr;

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<class UInputAction> TestLookAction = nullptr;

public:
    virtual void Destroy();
    virtual void ApplySpawnData(const FM1SpawnData& Data);
    virtual void ApplyStateData(EM1ActionType NewAction, uint8 NewInputMask);
};

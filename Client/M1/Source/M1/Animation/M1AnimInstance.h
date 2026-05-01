#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "M1AnimInstance.generated.h"

UCLASS()
class M1_API UM1AnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
    FORCEINLINE class AM1Character* GetOwnerCharacter() const
    {
        return OwnerCharacter.Get();
    }

protected:
    void UpdateOwner();
    void UpdateMovementData();
    void UpdateStateData();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim")
    TObjectPtr<class AM1Character> OwnerCharacter = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim")
    TObjectPtr<class UCharacterMovementComponent> MovementComp = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Movement")
    uint8 MoveModeRaw = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Movement")
    float AnimSpeed = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Movement")
    float Direction = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Movement")
    bool bIsMoving = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Movement")
    bool bIsInAir = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|State")
    bool bIsDead = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|State")
    bool bCombatMode = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|State")
    uint8 ActionTypeRaw = 0;


};

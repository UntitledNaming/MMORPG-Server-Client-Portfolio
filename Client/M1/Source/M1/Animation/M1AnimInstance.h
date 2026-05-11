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

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim")
    TObjectPtr<class AM1Character> OwnerCharacter = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Movement")
    float MoveSpeed = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Movement")
    bool bIsMoving = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|State")
    bool bIsDead = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|State")
    bool bCombatMode = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|State")
    bool bIsJumping = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|State")
    bool bSpanwend = false;

    // 피격 방향 각도 (-180 ~ 180): BS_HitReact X축
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Combat")
    float HitDirectionAngle = 0.f;

    // 피격 상태: AnimBP 상태머신 전이 조건
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsHit = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Combat")
    bool bUseUpperBodyWhenMoving = false;

};

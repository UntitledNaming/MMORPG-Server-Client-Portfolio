#pragma once

#include "CoreMinimal.h"
#include "Animation/M1AnimInstance.h"
#include "M1PlayerAnimInstance.generated.h"

UCLASS()
class M1_API UM1PlayerAnimInstance : public UM1AnimInstance
{
	GENERATED_BODY()

public:
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Player")
    bool bIsAttacking = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Player")
    bool bIsUsingSkill = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Player")
    bool bIsRunning = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Player")
    bool bJumpRequest = false;

protected:
    void UpdatePlayerData();
};

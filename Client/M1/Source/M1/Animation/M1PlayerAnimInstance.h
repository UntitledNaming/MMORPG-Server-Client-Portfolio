#pragma once

#include "CoreMinimal.h"
#include "Animation/M1AnimInstance.h"
#include "M1PlayerAnimInstance.generated.h"

UCLASS()
class M1_API UM1PlayerAnimInstance : public UM1AnimInstance
{
	GENERATED_BODY()

public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;


protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Movement")
    uint8 MoveMode = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|State")
    uint8 ActionType = 0;

private:
    TObjectPtr<class AM1BasePlayer> OwnerPlayer;
};

#pragma once

#include "CoreMinimal.h"
#include "Animation/M1AnimInstance.h"
#include "M1MonsterAnimInstance.generated.h"

UCLASS()
class M1_API UM1MonsterAnimInstance : public UM1AnimInstance
{
	GENERATED_BODY()

public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;


protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Movement")
    float MoveDirection = 0;

private:
    TObjectPtr<class AM1Monster> Owner;
};

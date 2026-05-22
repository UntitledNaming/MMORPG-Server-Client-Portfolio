#include "M1PlayerAnimInstance.h"
#include "Character/M1Character.h"
#include "Character\M1BasePlayer.h"

void UM1PlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation(); 

}


void UM1PlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (OwnerPlayer == nullptr)
    {
        OwnerPlayer = Cast<AM1BasePlayer>(GetOwningActor());
        if (OwnerPlayer == nullptr)
            return;
    }

    MoveDirection = OwnerPlayer->GetMoveDirection();
    bUseUpperBodyWhenMoving = OwnerPlayer->GetUseUpperBodyWhenMovingFlag();
}


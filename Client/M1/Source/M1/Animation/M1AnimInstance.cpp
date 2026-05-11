#include "M1AnimInstance.h"
#include "ContentsDefine.h"
#include "Character/M1Character.h"
#include "KismetAnimationLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

void UM1AnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

}

void UM1AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (OwnerCharacter == nullptr)
    {
        OwnerCharacter = Cast<AM1Character>(GetOwningActor());
        if (OwnerCharacter == nullptr)
            return;
    }

    MoveSpeed  = OwnerCharacter->GetMoveSpeed();
    bIsDead    = OwnerCharacter->IsDead();
    bIsJumping = OwnerCharacter->GetCharacterMovement()->IsFalling();
    bIsMoving  = OwnerCharacter->GetMoveFlag();
    bSpanwend  = OwnerCharacter->GetSpawnFlag();

    HitDirectionAngle = OwnerCharacter->GetHitDirectionAngle();
    bIsHit            = OwnerCharacter->IsHit();
}

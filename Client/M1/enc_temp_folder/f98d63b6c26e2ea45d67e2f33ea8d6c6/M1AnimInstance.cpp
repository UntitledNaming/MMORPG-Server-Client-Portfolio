#include "M1AnimInstance.h"
#include "ContentsDefine.h"
#include "Character/M1Character.h"
#include "Character\M1Player.h"
#include "KismetAnimationLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

void UM1AnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    UpdateOwner();
}

void UM1AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    UpdateOwner();

    if (OwnerCharacter == nullptr || MovementComp == nullptr)
        return;

    UpdateMovementData();
    UpdateStateData();
}


void UM1AnimInstance::UpdateOwner()
{
    if (OwnerCharacter != nullptr && MovementComp != nullptr)
        return;

    APawn* Pawn = TryGetPawnOwner();
    if (Pawn == nullptr)
        return;

    OwnerCharacter = Cast<AM1Character>(Pawn);
    if (OwnerCharacter == nullptr || !OwnerCharacter->GetIsSpawnInit())
        return;

    MovementComp = OwnerCharacter->GetCharacterMovement();
}

void UM1AnimInstance::UpdateMovementData()
{
    if (OwnerCharacter->bIsMyPlayer)
    {
        FVector Velocity = OwnerCharacter->GetVelocity();
        Velocity.Z = 0.f;

        AnimSpeed = Velocity.Size();
        bIsMoving = (AnimSpeed > 3.f);
        bIsInAir = MovementComp->IsFalling();
        Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, OwnerCharacter->GetActorRotation());

        if (bIsMoving == false)
        {
            Direction = 0.f;
            return;
        }

        AM1Player* Player = Cast<AM1Player>(OwnerCharacter);
        if (Player != nullptr)
        {
            MoveModeRaw = Player->GetMoveMode();
        }
    }
    else
    {
        bIsMoving = OwnerCharacter->GetRenderMove();
        AnimSpeed = bIsMoving ? OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed : 0.f;
    }
}

void UM1AnimInstance::UpdateStateData()
{
    bIsDead = OwnerCharacter->IsDeadState();
    ActionTypeRaw = OwnerCharacter->GetActionType();
}

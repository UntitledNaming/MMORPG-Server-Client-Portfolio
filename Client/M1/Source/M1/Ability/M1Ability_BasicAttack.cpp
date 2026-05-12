// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/M1Ability_BasicAttack.h"
#include "Character\M1Character.h"
#include "Controller\M1PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"

void UM1Ability_BasicAttack::OnActivate(AM1Character* Owner)
{
    if (!Owner || bIsAttacking) return;
    bIsAttacking = true;
    CachedOwner  = Owner;

    Owner->SetUseUpperBodyWhenMovingFlag(true);

    if (UCharacterMovementComponent* MoveComp = Owner->GetCharacterMovement())
        MoveComp->bOrientRotationToMovement = false;
    Owner->bUseControllerRotationYaw = true;

    PlayCastFX(Owner);

    UAnimInstance* AnimInst = Owner->GetMesh()->GetAnimInstance();
    if (AnimInst)
        AnimInst->OnMontageEnded.AddDynamic(this, &UM1Ability_BasicAttack::OnMontageEnded);
}

void UM1Ability_BasicAttack::OnDeactivate(AM1Character* Owner)
{
    if (!Owner || !bIsAttacking) return;
    bIsAttacking = false;
    CachedOwner  = nullptr;

    Owner->SetUseUpperBodyWhenMovingFlag(false);

    if (UCharacterMovementComponent* MoveComp = Owner->GetCharacterMovement())
        MoveComp->bOrientRotationToMovement = true;
    Owner->bUseControllerRotationYaw = false;

    UAnimInstance* AnimInst = Owner->GetMesh()->GetAnimInstance();
    if (AnimInst)
    {
        AnimInst->OnMontageEnded.RemoveDynamic(this, &UM1Ability_BasicAttack::OnMontageEnded);
        if (FXData.CastMontage)
            AnimInst->Montage_Stop(0.15f, FXData.CastMontage);
    }

    AM1PlayerController* PC = Cast<AM1PlayerController>(Owner->GetController());
    if (PC)
        PC->SendLeftAttackStopPacket();
}

void UM1Ability_BasicAttack::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (bInterrupted || !bIsAttacking || !CachedOwner) return;
    if (Montage != FXData.CastMontage) return;

    PlayCastFX(CachedOwner);
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/M1Ability_BasicAttack.h"
#include "ContentsDefine.h"
#include "Character\M1Character.h"
#include "Controller\M1PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"

UM1Ability_BasicAttack::UM1Ability_BasicAttack()
{
    FXData.CastEffectRadius = ClientAttack::LEFTATTACK_RANGE;
}

void UM1Ability_BasicAttack::OnActivate(AM1Character* Owner)
{
    if (!Owner || Owner->IsDead() || bIsAttacking || bPendingStop)
        return;

    bIsAttacking = true;
    CachedOwner  = Owner;

    if (UCharacterMovementComponent* MoveComp = Owner->GetCharacterMovement())
        MoveComp->bOrientRotationToMovement = false;
    Owner->bUseControllerRotationYaw = true;

    PlayCastFX(Owner);

    UAnimInstance* AnimInst = Owner->GetMesh()->GetAnimInstance();
    if (AnimInst)
    {
        AnimInst->OnMontageEnded.RemoveDynamic(this, &UM1Ability_BasicAttack::OnMontageEnded);
        AnimInst->OnMontageEnded.AddDynamic(this, &UM1Ability_BasicAttack::OnMontageEnded);
    }
}

void UM1Ability_BasicAttack::OnDeactivate(AM1Character* Owner)
{
    if (!Owner || !bIsAttacking)
        return;

    bPendingStop = true;

    UAnimInstance* AnimInst = Owner->GetMesh()->GetAnimInstance();
    if (AnimInst && FXData.CastMontage)
    {
        if (FAnimMontageInstance* Inst = AnimInst->GetActiveMontageInstance())
        {
            FName CurSection = Inst->GetCurrentSection();
            if (!CurSection.IsNone())
                AnimInst->Montage_SetNextSection(CurSection, NAME_None, FXData.CastMontage);
        }
    }
}

void UM1Ability_BasicAttack::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage != FXData.CastMontage) return;

    if (bPendingStop || bInterrupted)
    {
        bIsAttacking = false;
        bPendingStop = false;

        if (CachedOwner)
        {
            if (UCharacterMovementComponent* MoveComp = CachedOwner->GetCharacterMovement())
                MoveComp->bOrientRotationToMovement = true;
            CachedOwner->bUseControllerRotationYaw = false;

            UAnimInstance* AnimInst = CachedOwner->GetMesh()->GetAnimInstance();
            if (AnimInst)
                AnimInst->OnMontageEnded.RemoveDynamic(this, &UM1Ability_BasicAttack::OnMontageEnded);

            AM1PlayerController* PC = Cast<AM1PlayerController>(CachedOwner->GetController());
            if (PC) PC->SendLeftAttackStopPacket();
        }

        CachedOwner = nullptr;
        return;
    }

    if (bIsAttacking && CachedOwner && !CachedOwner->IsDead())
        PlayCastFX(CachedOwner);
}

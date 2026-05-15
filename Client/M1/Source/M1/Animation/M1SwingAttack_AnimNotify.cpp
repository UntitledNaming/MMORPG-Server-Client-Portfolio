// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/M1SwingAttack_AnimNotify.h"
#include "Character\M1LocalPlayer.h"
#include "Controller\M1PlayerController.h"
#include "Ability\M1PlayerActorComponent.h"

void UM1SwingAttack_AnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    AM1LocalPlayer* Character = Cast<AM1LocalPlayer>(MeshComp->GetOwner());
    if (!Character) return;

    if (UM1PlayerActorComponent* AC = Character->GetAbilityComponent())
        if (AC->IsBasicAttackPendingStop()) return;

    AM1PlayerController* PC = Cast<AM1PlayerController>(Character->GetController());
    if (!PC) return;

    PC->SendLeftAttackSwingPacket(PC->GetControlRotation().Yaw, SwingIndex);
}
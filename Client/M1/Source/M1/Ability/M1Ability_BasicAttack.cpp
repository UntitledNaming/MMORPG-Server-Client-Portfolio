// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/M1Ability_BasicAttack.h"
#include "Engine/GameInstance.h"
#include "Character\M1Character.h"
#include "Controller\M1PlayerController.h"
#include "System\M1SpawnManager.h"
#include "Network\M1NetworkManager.h"
#include "GameFramework/CharacterMovementComponent.h"

// Ability_BasicAttack - 즉시 실행형
void UM1Ability_BasicAttack::OnActivate(AM1Character* Owner)
{
    if (!Owner || bIsAttacking) return;
    bIsAttacking = true;
    Owner->SetUseUpperBodyWhenMovingFlag(true);

    if (UCharacterMovementComponent* MoveComp = Owner->GetCharacterMovement())
        MoveComp->bOrientRotationToMovement = false;
    Owner->bUseControllerRotationYaw = true;

    AM1PlayerController* PC = Cast<AM1PlayerController>(Owner->GetController());
    if (PC)
        PC->SendLeftAttackStartPacket(PC->GetControlRotation().Yaw);

    PerformAttack(Owner,PC->GetNetworkManager());
    Owner->GetWorldTimerManager().SetTimer(
        AttackTimerHandle,
        [this, Owner, PC]() { PerformAttack(Owner, PC->GetNetworkManager()); },
        AttackInterval, true);
}

void UM1Ability_BasicAttack::OnDeactivate(AM1Character* Owner)
{
    if (!Owner || !bIsAttacking) return;
    bIsAttacking = false;
    Owner->SetUseUpperBodyWhenMovingFlag(false);

    if (UCharacterMovementComponent* MoveComp = Owner->GetCharacterMovement())
        MoveComp->bOrientRotationToMovement = true;
    Owner->bUseControllerRotationYaw = false;

    Owner->GetWorldTimerManager().ClearTimer(AttackTimerHandle);

    AM1PlayerController* PC = Cast<AM1PlayerController>(Owner->GetController());
    if (PC)
        PC->SendLeftAttackStopPacket();


    // 몽타주 재생중이면 정지
    if (FXData.CastMontage)
    {
        UAnimInstance* AnimInst = Owner->GetMesh()->GetAnimInstance();

        AnimInst->Montage_Stop(0.15f, FXData.CastMontage);
    }

}

void UM1Ability_BasicAttack::PerformAttack(AM1Character* Owner, class UM1NetworkManager* NetworkManager)
{
    if (!Owner || NetworkManager == nullptr) return;

    PlayCastFX(Owner);

    if (AM1SpawnManager* SM = NetworkManager->GetSpawnManager())
    {
        SM->ProcessClientSingleTargetHit(Owner->GetActorLocation(), Owner->GetActorForwardVector(), AttackRange, AttackHalfAngle);
    }
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/M1GroundSmash_AnimNotify.h"
#include "Character\M1LocalPlayer.h"
#include "Character\M1BasePlayer.h"
#include "Controller\M1PlayerController.h"
#include "System\M1SpawnManager.h"
#include "Network\M1NetworkManager.h"
#include "Engine\GameInstance.h"
#include "ContentsDefine.h"
#include "Ability/M1PlayerActorComponent.h"

void UM1GroundSmash_AnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    AM1BasePlayer* Character = Cast<AM1BasePlayer>(MeshComp->GetOwner());
    if (!Character) return;

    UGameInstance* GI = Character->GetGameInstance();
    UM1NetworkManager* NetworkManager = GI ? GI->GetSubsystem<UM1NetworkManager>() : nullptr;
    if (!NetworkManager) return;

    AM1SpawnManager* SM = NetworkManager->GetSpawnManager();
    if (!SM) return;

    UM1PlayerActorComponent* Comp = Character->GetAbilityComponent();
    Comp->TriggerImpactFX(EAbilitySlot::Skill3);

    SM->ProcessClientAttackHit(Character->GetActorLocation(), Character->GetActorForwardVector(), ClientAttack::GROUNDSMASH_RANGE, 180.f);
}
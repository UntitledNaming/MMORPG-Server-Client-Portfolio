// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/M1AbilityBase.h"
#include "Character\M1Character.h"
#include "Network\M1NetworkManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

void UM1AbilityBase::PlayCastFX(AM1Character* Owner)
{
    if (!Owner) return;

    if (FXData.CastMontage)
    {
        UAnimInstance* AnimInst = Owner->GetMesh()->GetAnimInstance();

        // 이미 재생 중이면 스킵
        if (AnimInst && !AnimInst->Montage_IsPlaying(FXData.CastMontage))
            Owner->PlayAnimMontage(FXData.CastMontage, FXData.CastMontagePlayRate);
    }

    USkeletalMeshComponent* MeshComp = Owner->GetMesh();
    if (MeshComp == nullptr)
        return;

    if (FXData.CastEffect)
    {
        FVector EffectScale = FXData.CastEffectScale;
        if (FXData.CastEffectRadius > 0.f && FXData.CastEffectBaseRadius > 0.f)
        {
            float s = FXData.CastEffectRadius / FXData.CastEffectBaseRadius;
            EffectScale = FVector(s, s, s);
        }

        UGameplayStatics::SpawnEmitterAttached(
            FXData.CastEffect,
            MeshComp,
            NAME_None,
            FXData.CastEffectOffset,
            FXData.CastEffectRotation,
            EffectScale,
            EAttachLocation::KeepRelativeOffset,
            true
        );
    }

    if (FXData.CastSound)
        UGameplayStatics::PlaySoundAtLocation(
            Owner, FXData.CastSound, Owner->GetActorLocation());
}

void UM1AbilityBase::PlayHitFX(AM1Character* Target)
{
    if (!Target) return;

    USkeletalMeshComponent* MeshComp = Target->GetMesh();
    if (MeshComp == nullptr)
        return;

    if (FXData.HitEffect)
    {
        UGameplayStatics::SpawnEmitterAttached(FXData.HitEffect, MeshComp, FXData.HitEffectSocket, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
    }


    if (FXData.HitSound)
        UGameplayStatics::PlaySoundAtLocation(
            Target, FXData.HitSound, Target->GetActorLocation());
}

void UM1AbilityBase::PlayImpactFX(AM1Character* Owner)
{
    if (!Owner) return;

    USkeletalMeshComponent* MeshComp = Owner->GetMesh();
    if (!MeshComp) return;

    if (FXData.ImpactEffect)
        UGameplayStatics::SpawnEmitterAttached(
            FXData.ImpactEffect,
            MeshComp,
            NAME_None,
            FXData.ImpactEffectOffset,
            FRotator::ZeroRotator,
            FXData.ImpactEffectScale,
            EAttachLocation::KeepRelativeOffset,
            true);
}

UM1NetworkManager* UM1AbilityBase::GetNetworkManager(AM1Character* Owner) const
{
    if (!Owner) return nullptr;
    UGameInstance* GI = Owner->GetGameInstance();
    return GI ? GI->GetSubsystem<UM1NetworkManager>() : nullptr;
}
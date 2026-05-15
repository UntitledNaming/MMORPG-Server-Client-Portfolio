// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "Animation/AnimMontage.h"
#include "M1AbilityTypes.generated.h"

UENUM(BlueprintType)
enum class EAbilitySlot : uint8
{
    BasicAttack UMETA(DisplayName = "Basic Attack"),
    Skill1      UMETA(DisplayName = "Skill 1"),
    Skill2      UMETA(DisplayName = "Skill 2"),
    Skill3      UMETA(DisplayName = "Skill 3"),
    Skill4      UMETA(DisplayName = "Skill 4"),
};

///////////////////////////////////////////////////////
//  1번 스킬 Particle : GroundSmash1
//  2번 스킬 Particle : SwordSpinning Start
//  3번 스킬 Particle : GroundSmash or Location
//  4번 스킬 Particel : Ultimate 폴더 Particle 여러개 
///////////////////////////////////////////////////////


USTRUCT(BlueprintType)
struct FAbilityFXData
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    UAnimMontage* CastMontage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
    UParticleSystem* CastEffect = nullptr;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
    UParticleSystem* HitEffect = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
    UParticleSystem* ImpactEffect = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
    USoundBase* CastSound = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
    USoundBase* HitSound = nullptr;
};
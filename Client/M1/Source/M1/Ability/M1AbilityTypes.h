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

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    float CastMontagePlayRate = 1.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
    UParticleSystem* CastEffect = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX|Cast")
    FVector CastEffectOffset = FVector(0.f, 0.f, 100.f);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX|Cast")
    FRotator CastEffectRotation = FRotator::ZeroRotator;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX|Cast")
    FVector CastEffectScale = FVector(2.f, 2.f, 2.f);

    // 0 이상이면 CastEffectScale 무시하고 Radius / BaseRadius 로 스케일 자동 계산
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX|Cast")
    float CastEffectRadius = 0.f;

    // 이 파티클이 스케일 (1,1,1)일 때 커버하는 반경(cm) — BP에서 파티클별로 캘리브레이션
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX|Cast")
    float CastEffectBaseRadius = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
    UParticleSystem* HitEffect = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX|Hit")
    FName HitEffectSocket = TEXT("spine_01");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
    UParticleSystem* ImpactEffect = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX|Impact")
    FVector ImpactEffectOffset = FVector::ZeroVector;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX|Impact")
    FVector ImpactEffectScale = FVector(1.f, 1.f, 1.f);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
    USoundBase* CastSound = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
    USoundBase* HitSound = nullptr;
};
// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/M1MonsterAnimInstance.h"
#include "Character/M1Monster.h"

void UM1MonsterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

}


void UM1MonsterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (Owner == nullptr)
    {
        Owner = Cast<AM1Monster>(GetOwningActor());
        if (Owner == nullptr)
            return;
    }

    MoveDirection = Owner->GetActorRotation().Yaw;
    bUseUpperBodyWhenMoving = Owner->GetUseUpperBodyWhenMovingFlag();
}
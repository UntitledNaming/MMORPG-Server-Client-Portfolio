#include "M1PlayerAnimInstance.h"
#include "ContentsEnum.h"
#include "Character/M1Character.h"
#include "Character\M1Player.h"

void UM1PlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (OwnerCharacter == nullptr)
        return;

    UpdatePlayerData();
}

void UM1PlayerAnimInstance::UpdatePlayerData()
{
    EM1ActionStateType CurrentAction = static_cast<EM1ActionStateType>(ActionTypeRaw);
    EM1MoveMode CurrentMoveMode = static_cast<EM1MoveMode>(MoveModeRaw);

    bIsAttacking = (CurrentAction == EM1ActionStateType::Attack);
    bIsUsingSkill = (CurrentAction == EM1ActionStateType::Skill);
    bIsRunning = (CurrentMoveMode == EM1MoveMode::Run);
    bCombatMode = (CurrentAction == EM1ActionStateType::Attack || CurrentAction == EM1ActionStateType::Skill);

    AM1Player* Player = Cast<AM1Player>(OwnerCharacter);
    if (Player)
    {
        bJumpRequest = Player->GetJumpRequest();
    }
}
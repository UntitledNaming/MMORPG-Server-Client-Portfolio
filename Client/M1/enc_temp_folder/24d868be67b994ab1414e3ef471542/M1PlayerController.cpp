#include "M1PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/M1Player.h"
#include "System/M1AssetManager.h"
#include "Data/M1InputDataAsset.h"
#include "Data/M1PrimaryDataAsset.h"
#include "M1GameplayTags.h"
#include "InputActionValue.h"


AM1PlayerController::AM1PlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void AM1PlayerController::SetCachedPlayer(AM1Player* InPlayer)
{
    M1Player = InPlayer;
}

void AM1PlayerController::BeginPlayingState()
{
    Super::BeginPlayingState();

    //if (const UM1InputDataAsset* InputData = UM1AssetManager::GetAssetByName<UM1InputDataAsset>("InputDataAsset"))
    //{
    //    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    //    {
    //        Subsystem->AddMappingContext(InputData->InputMappingContext, 0);

    //        if (UEnhancedPlayerInput* EPI = Cast<UEnhancedPlayerInput>(PlayerInput))
    //        {
    //            auto JumpAction = InputData->FindInputActionByTag(M1GameplayTags::Input_Action_Jump);
    //            auto MoveAction = InputData->FindInputActionByTag(M1GameplayTags::Input_Action_Move);
    //            auto LookAction = InputData->FindInputActionByTag(M1GameplayTags::Input_Action_Look);
    //            auto LeftAttackAction = InputData->FindInputActionByTag(M1GameplayTags::Input_Action_LeftAttack);

    //            EPI->InjectInputForAction(JumpAction, FInputActionValue(true));
    //            EPI->InjectInputForAction(MoveAction, FInputActionValue(true));
    //            EPI->InjectInputForAction(LookAction, FInputActionValue(true));
    //            EPI->InjectInputForAction(LeftAttackAction, FInputActionValue(true));
    //        }
    //    }
    //}


    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->ClearAllMappings();
        Subsystem->AddMappingContext(TestIMC, 0);
        Subsystem->RequestRebuildControlMappings();
    }
}

void AM1PlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();


    //if (const UM1InputDataAsset* InputData = UM1AssetManager::GetAssetByName<UM1InputDataAsset>("InputDataAsset"))
    //{

    //    UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

    //    auto JumpAction = InputData->FindInputActionByTag(M1GameplayTags::Input_Action_Jump);
    //    auto MoveAction = InputData->FindInputActionByTag(M1GameplayTags::Input_Action_Move);
    //    auto LookAction = InputData->FindInputActionByTag(M1GameplayTags::Input_Action_Look);
    //    auto LeftAttackAction = InputData->FindInputActionByTag(M1GameplayTags::Input_Action_LeftAttack);

    //    EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AM1PlayerController::OnJumpStart);
    //    EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AM1PlayerController::OnJumpEnd);
    //    EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AM1PlayerController::OnMove);
    //    EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AM1PlayerController::OnLook);
    //    EnhancedInputComponent->BindAction(LeftAttackAction, ETriggerEvent::Started, this, &AM1PlayerController::OnAttackStart);
    //    EnhancedInputComponent->BindAction(LeftAttackAction, ETriggerEvent::Completed, this, &AM1PlayerController::OnAttackEnd);
    //    EnhancedInputComponent->BindAction(LeftAttackAction, ETriggerEvent::Canceled, this, &AM1PlayerController::OnAttackEnd);


    //    EnhancedInputComponent->BindAction(TestJumpAction, ETriggerEvent::Started, this, &AM1PlayerController::OnJumpStart);
    //    EnhancedInputComponent->BindAction(TestJumpAction, ETriggerEvent::Completed, this, &AM1PlayerController::OnJumpEnd);

    //}


    UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(InputComponent);

    EIC->BindAction(TestJumpAction, ETriggerEvent::Started, this, &AM1PlayerController::OnJumpStart);
    EIC->BindAction(TestJumpAction, ETriggerEvent::Completed, this, &AM1PlayerController::OnJumpEnd);
}

void AM1PlayerController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);
}

void AM1PlayerController::OnMove(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();

    DoMove(MovementVector.X, MovementVector.Y);
}

void AM1PlayerController::OnLook(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AM1PlayerController::OnJumpStart()
{
    if (M1Player)
    {
        M1Player->Jump();
    }
}

void AM1PlayerController::OnJumpEnd()
{
    if (M1Player)
    {
        M1Player->StopJumping();
    }
}

void AM1PlayerController::OnAttackStart()
{
    bLeftMousePressed = true;
}

void AM1PlayerController::OnAttackEnd()
{
    bLeftMousePressed = false;
}

void AM1PlayerController::DoMove(float Right, float Forward)
{
    const FRotator Rotation = GetControlRotation();
    const FRotator YawRotation(0, Rotation.Yaw, 0);

    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    if (M1Player)
    {
        M1Player->AddMovementInput(ForwardDirection, Forward);
        M1Player->AddMovementInput(RightDirection, Right);
    }
}

void AM1PlayerController::DoLook(float Yaw, float Pitch)
{
    if (M1Player)
    {
        M1Player->AddControllerYawInput(Yaw);
        M1Player->AddControllerPitchInput(Pitch);
    }
}


#include "M1PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Network\M1NetworkManager.h"
#include "System\M1SpawnManager.h"
#include "Network\ClientCore\CMessage.h"
#include "NetPacketHeader.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character\M1LocalPlayer.h"
#include "System/M1AssetManager.h"
#include "Data/M1InputDataAsset.h"
#include "Data/M1PrimaryDataAsset.h"
#include "Engine/GameInstance.h"
#include "ContentsDefine.h"
#include "ContentsProtocol.h"
#include "M1GameplayTags.h"
#include "Ability\M1AbilityTypes.h"
#include "InputActionValue.h"
#include "Components/WidgetComponent.h"
#include "UI\M1MainHUDWidget.h"

AM1PlayerController::AM1PlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void AM1PlayerController::SetCachedPlayer(AM1LocalPlayer* InPlayer)
{
    M1Player = InPlayer;
}

void AM1PlayerController::SetLastYaw(float Yaw)
{
    LastSendYaw = Yaw;
}

UM1NetworkManager* AM1PlayerController::GetNetworkManager()
{
    return NetworkManager;
}


void AM1PlayerController::BeginPlayingState()
{
    Super::BeginPlayingState();

    if (!NetworkManager)
    {
        if (UGameInstance* GI = GetGameInstance())
        {
            NetworkManager = GI->GetSubsystem<UM1NetworkManager>();
        }
    }
}

void AM1PlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();


    if (const UM1InputDataAsset* InputData = UM1AssetManager::GetAssetByName<UM1InputDataAsset>("InputDataAsset"))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(InputData->InputMappingContext, 0);
        }

        UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

        auto JumpAction = InputData->FindInputActionByTag(M1GameplayTags::Input_Action_Jump);
        auto MoveAction = InputData->FindInputActionByTag(M1GameplayTags::Input_Action_Move);
        auto LookAction = InputData->FindInputActionByTag(M1GameplayTags::Input_Action_Look);
        auto LeftAttackAction = InputData->FindInputActionByTag(M1GameplayTags::Input_Action_LeftAttack);
        auto Skill1Action = InputData->FindInputActionByTag(M1GameplayTags::Input_Action_Skill1);
        auto Skill2Action = InputData->FindInputActionByTag(M1GameplayTags::Input_Action_Skill2);
        auto Skill3Action = InputData->FindInputActionByTag(M1GameplayTags::Input_Action_Skill3);
        auto Skill4Action = InputData->FindInputActionByTag(M1GameplayTags::Input_Action_Skill4);
        auto PickUpAction = InputData->FindInputActionByTag(M1GameplayTags::Input_Action_PickUp);
        auto InventoryAction = InputData->FindInputActionByTag(M1GameplayTags::Input_Action_Inventory);
        auto EquipmentAction = InputData->FindInputActionByTag(M1GameplayTags::Input_Action_Equipment);

        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AM1PlayerController::OnMove);
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Canceled, this, &AM1PlayerController::OnMove);
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AM1PlayerController::OnMove);
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AM1PlayerController::OnLook);
        EnhancedInputComponent->BindAction(LeftAttackAction, ETriggerEvent::Started, this, &AM1PlayerController::OnStartLeftAttack);
        EnhancedInputComponent->BindAction(LeftAttackAction, ETriggerEvent::Triggered, this, &AM1PlayerController::OnLeftAttackHeld);
        EnhancedInputComponent->BindAction(LeftAttackAction, ETriggerEvent::Completed, this, &AM1PlayerController::OnStopLeftAttack);
        EnhancedInputComponent->BindAction(LeftAttackAction, ETriggerEvent::Canceled, this, &AM1PlayerController::OnStopLeftAttack);
        EnhancedInputComponent->BindAction(Skill1Action, ETriggerEvent::Started, this, &AM1PlayerController::OnUseSkill1);
        EnhancedInputComponent->BindAction(Skill2Action, ETriggerEvent::Started, this, &AM1PlayerController::OnUseSkill2);
        EnhancedInputComponent->BindAction(Skill3Action, ETriggerEvent::Started, this, &AM1PlayerController::OnUseSkill3);
        EnhancedInputComponent->BindAction(Skill4Action, ETriggerEvent::Started, this, &AM1PlayerController::OnUseSkill4);
        EnhancedInputComponent->BindAction(PickUpAction, ETriggerEvent::Started, this, &AM1PlayerController::OnPickUp);
        EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, this, &AM1PlayerController::ToggleInventory);
        EnhancedInputComponent->BindAction(EquipmentAction, ETriggerEvent::Started, this, &AM1PlayerController::ToggleEquipment);
    }

    InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AM1PlayerController::QuitGame);
}

void AM1PlayerController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);

    MovementSendTime += DeltaTime;

    TrySendMovementPacket();
}

void AM1PlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // 로컬 컨트롤러만 UI 생성 (서버에서는 UI 안 만듦)
    if (!IsLocalController())
        return;

    // 캐릭터 캐스팅
    AM1LocalPlayer* LocalPlayer = Cast<AM1LocalPlayer>(InPawn);
    if (!LocalPlayer)
        return;

    // HUD 클래스 설정 확인
    if (!MainHUDClass)
    {
        UE_LOG(LogTemp, Error, TEXT("MainHUDClass is not set in PlayerController!"));
        return;
    }

    // UI 위젯 생성
    MainHUD = CreateWidget<UUserWidget>(this, MainHUDClass);
    if (!MainHUD)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create MainHUD!"));
        return;
    }

    // 뷰포트에 추가
    MainHUD->AddToViewport();

    // M1MainHUDWidget으로 캐스팅해서 바인딩
    UM1MainHUDWidget* HUDWidget = Cast<UM1MainHUDWidget>(MainHUD);
    if (HUDWidget)
    {
        HUDWidget->BindToPlayer(LocalPlayer);
        UE_LOG(LogTemp, Log, TEXT("MainHUD created and bound to player"));
    }
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

void AM1PlayerController::OnStartLeftAttack()
{
    if (!M1Player)
        return;

    if (M1Player->GetAbilityComponent())
        M1Player->GetAbilityComponent()->ActivateAbility(EAbilitySlot::BasicAttack);
}

void AM1PlayerController::OnLeftAttackHeld()
{
    if (M1Player)
        M1Player->SetUseUpperBodyWhenMovingFlag(true);
}

void AM1PlayerController::OnStopLeftAttack()
{
    if (!M1Player)
        return;

    if (M1Player->GetAbilityComponent())
        M1Player->GetAbilityComponent()->DeactivateAbility(EAbilitySlot::BasicAttack);

    M1Player->SetUseUpperBodyWhenMovingFlag(false);
}

void AM1PlayerController::OnUseSkill1()
{
    if (M1Player && M1Player->GetAbilityComponent())
        M1Player->GetAbilityComponent()->ActivateAbility(EAbilitySlot::Skill1);
}

void AM1PlayerController::OnUseSkill2()
{
    if (M1Player && M1Player->GetAbilityComponent())
        M1Player->GetAbilityComponent()->ActivateAbility(EAbilitySlot::Skill2);
}

void AM1PlayerController::OnUseSkill3()
{
    if (M1Player && M1Player->GetAbilityComponent())
        M1Player->GetAbilityComponent()->ActivateAbility(EAbilitySlot::Skill3);
}

void AM1PlayerController::OnUseSkill4()
{
    if (M1Player && M1Player->GetAbilityComponent())
        M1Player->GetAbilityComponent()->ActivateAbility(EAbilitySlot::Skill4);
}

void AM1PlayerController::OnPickUp()
{
    if (M1Player == nullptr)
        return;

    AM1SpawnManager* SpawnManager = NetworkManager->GetSpawnManager();
    if (SpawnManager == nullptr)
        return;

    SpawnManager->TryPickUpNearestItem(M1Player->GetActorLocation());
}

// 매개인자 : WASD 입력에 대한 X, Y축값
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

    // 이동 벡터 계산
    FVector MoveDirection = ForwardDirection * Forward + RightDirection * Right;
    MoveDirection.Z = 0.f;

    // 이동 벡터 크기가 0에 가까우면 멈춤으로 판단해서 CurrentMoveFlag를 false로 변경
    bCurrentMoveFlag = !(FMath::IsNearlyZero(Right) && FMath::IsNearlyZero(Forward));

    if (bCurrentMoveFlag)
    {
        MoveDirection.Normalize();

        CurrentYaw = MoveDirection.Rotation().Yaw;
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

void AM1PlayerController::QuitGame()
{
    UKismetSystemLibrary::QuitGame(
        this,
        this,
        EQuitPreference::Quit,
        true
    );
}

void AM1PlayerController::TrySendMovementPacket()
{
    if (M1Player == nullptr || NetworkManager == nullptr)
        return;

    bool bNeedSend = false;

    // 1) 이동 시작, 이동 중 정지 할때 패킷 무조건 보내야 하니 bNeedSend 플래그 true 설정
    if (bCurrentMoveFlag != bLastSendMoveFlag)
    {
        bNeedSend = true;
    }

    // 2) 그게 아니라면 계속 이동중인 상황에서 방향이 임계값 넘어가면 true 설정
    else if (bCurrentMoveFlag)
    {
        float DeltaYaw = FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentYaw, LastSendYaw));

        if (DeltaYaw >= ClientMovement::MOVEMENT_YAW_SEND_THRESHOLD_DEG)
        {
            bNeedSend = true;
        }
    }

    // 3) 아직 보낼 플래그는 안켜졌고 현재 이동 중이며 만약 시간이 0.1초 이상 지났다면 보낼 것임.
    if (!bNeedSend && bCurrentMoveFlag && MovementSendTime >= ClientMovement::MOVEMENT_SEND_INTERNAL_SEC)
    {
        bNeedSend = true;
    }

    if (!bNeedSend)
        return;

    const FVector Pos = M1Player->GetActorLocation();

    // 메세지 만들기
    CMessage* pMessage = CMessage::Alloc();
    pMessage->Clear(1);

    mpMovementInput(pMessage, Pos, CurrentYaw, bCurrentMoveFlag);

    NetworkManager->SendPacket(pMessage, static_cast<uint8>(ERouteType::GROUP), ServiceID::NONE_SERVICE);
    CMessage::Free(pMessage);

    bLastSendMoveFlag = bCurrentMoveFlag;
    LastSendYaw = CurrentYaw;
    MovementSendTime = 0.0f;

}

void AM1PlayerController::ToggleInventory()
{
    UM1MainHUDWidget* HUDWidget = Cast<UM1MainHUDWidget>(MainHUD);
    if (!HUDWidget)
        return;

    HUDWidget->ToggleInventoryPanel();
    RefreshItemUIInputMode(HUDWidget->IsAnyItemPanelOpen());
}

void AM1PlayerController::ToggleEquipment()
{
    UM1MainHUDWidget* HUDWidget = Cast<UM1MainHUDWidget>(MainHUD);
    if (!HUDWidget)
        return;

    HUDWidget->ToggleEquipmentPanel();
    RefreshItemUIInputMode(HUDWidget->IsAnyItemPanelOpen());
}

void AM1PlayerController::RefreshItemUIInputMode(bool bOpen)
{
    bShowMouseCursor = bOpen;
    bEnableClickEvents = bOpen;
    bEnableMouseOverEvents = bOpen;

    if (bOpen)
    {
        FInputModeGameAndUI Mode;
        Mode.SetHideCursorDuringCapture(false);
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        SetInputMode(Mode);
    }
    else
    {
        FInputModeGameOnly Mode;
        SetInputMode(Mode);
    }
}

void AM1PlayerController::mpMovementInput(CMessage* pMessage, const FVector& Location, float Yaw, bool MoveFlag)
{
    *pMessage << FieldProtocol::PACKET_CS_UPDATE_CHARACTER_MOVEMENT_INPUT;
    *pMessage << (float)Location.X;
    *pMessage << (float)Location.Y;
    *pMessage << (float)Location.Z;
    *pMessage << Yaw;
    *pMessage << MoveFlag;
}

void AM1PlayerController::SendLeftAttackSwingPacket(float FacingYaw, uint8 SwingIdx)
{
    if (!NetworkManager)
        return;

    CMessage* pMessage = CMessage::Alloc();
    pMessage->Clear(1);
    *pMessage << FieldProtocol::PACKET_CS_SWING_LEFT_ATTACK;
    *pMessage << FacingYaw;
    *pMessage << SwingIdx;
    NetworkManager->SendPacket(pMessage, static_cast<uint8>(ERouteType::GROUP), ServiceID::NONE_SERVICE);
    CMessage::Free(pMessage);
}

void AM1PlayerController::SendLeftAttackStopPacket()
{
    if (!NetworkManager)
        return;

    CMessage* pMessage = CMessage::Alloc();
    pMessage->Clear(1);
    *pMessage << FieldProtocol::PACKET_CS_STOP_LEFT_ATTACK;
    NetworkManager->SendPacket(pMessage, static_cast<uint8>(ERouteType::GROUP), ServiceID::NONE_SERVICE);
    CMessage::Free(pMessage);
}

void AM1PlayerController::SendUseSkillPacket(uint8 SkillSlot)
{
    if (!NetworkManager)
        return;

    CMessage* pMessage = CMessage::Alloc();
    pMessage->Clear(1);
    *pMessage << FieldProtocol::PACKET_CS_USE_SKILL;
    *pMessage << SkillSlot;
    NetworkManager->SendPacket(pMessage, static_cast<uint8>(ERouteType::GROUP), ServiceID::NONE_SERVICE);
    CMessage::Free(pMessage);
}

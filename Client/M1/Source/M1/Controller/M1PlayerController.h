#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "M1PlayerController.generated.h"

struct FInputActionValue;
class AM1LocalPlayer;
class CMessage;
class UM1NetworkManager;

UCLASS()
class M1_API AM1PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AM1PlayerController(const FObjectInitializer& ObjectInitializer);

	void SetCachedPlayer(AM1LocalPlayer* InPlayer);
	void SetLastYaw(float Yaw);
	UM1NetworkManager* GetNetworkManager();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;

public:
	void OnPossess(APawn* InPawn);
	
	void ShowLoginWidget();
	void CloseLoginWidget();

	void ShowRespawnWidget();
	void CloseRespawnWidget();

private:
	void OnMove(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnJumpStart();
	void OnJumpEnd();
	void OnStartLeftAttack();
	void OnLeftAttackHeld();
	void OnStopLeftAttack();
	void OnUseSkill1();
	void OnUseSkill2();
	void OnUseSkill3();
	void OnUseSkill4();
	void OnPickUp();
	void OnQuickSlot1();
	void OnQuickSlot2();
	void DoMove(float Right, float Forward);
	void DoLook(float Yaw, float Pitch);
	void QuitGame();

	void TrySendMovementPacket();

	void ToggleInventory();
	void ToggleEquipment();
	void RefreshItemUIInputMode(bool bOpen);

public:
	void SendLeftAttackSwingPacket(float FacingYaw, uint8 SwingIdx);
	void SendUseSkillPacket(uint8 SkillSlot);

private:
	void mpMovementInput(CMessage* pMessage, const FVector& Location, float Yaw, bool MoveFlag);

protected:
	UPROPERTY(BlueprintReadOnly)                 TObjectPtr<AM1LocalPlayer> M1Player = nullptr;

	UPROPERTY()                                  TObjectPtr<UM1NetworkManager> NetworkManager = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "UI") TSubclassOf<class UUserWidget> MainHUDClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI") TSubclassOf<class UUserWidget> LoginWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI") TSubclassOf<class UUserWidget> RespawnWidgetClass;

	UPROPERTY()                                  class UUserWidget* MainHUD = nullptr;
	UPROPERTY()                                	 class UUserWidget* LoginWidget = nullptr;
	UPROPERTY()                                  class UUserWidget* RespawnWidget = nullptr;



private:
	bool   bCurrentMoveFlag  = false;
	bool   bLastSendMoveFlag = false;
	float  MovementSendTime  = 0.0f;
	float  LastSendYaw       = 0.0f;
	float  CurrentYaw        = 0.0f;
	double LastSendTime      = 0;

};

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "M1PlayerController.generated.h"

struct FInputActionValue;
class AM1Player;
class CMessage;

UCLASS()
class M1_API AM1PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AM1PlayerController(const FObjectInitializer& ObjectInitializer);

	void SetCachedPlayer(AM1Player* InPlayer);
	void SetLastYaw(float Yaw);

protected:
	virtual void BeginPlayingState() override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;

private:
	void OnMove(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnJumpStart();
	void OnJumpEnd();
	void OnAttackStart();
	void OnAttackEnd();
	void DoMove(float Right, float Forward);
	void DoLook(float Yaw, float Pitch);

	void TrySendMovementPacket();

private:
	void mpMovementInput(CMessage* pMessage, const FVector& Location, float Yaw, bool MoveFlag);

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AM1Player> M1Player;

	UPROPERTY()
	TObjectPtr<class UM1NetworkManager> NetworkManager = nullptr;

	bool bCurrentMoveFlag = false;
	bool bLastSendMoveFlag = false;
	float MovementSendTime = 0.0f;
	float LastSendYaw = 0.0f;
	float CurrentYaw = 0.0f;
	double LastSendTime = 0;
};

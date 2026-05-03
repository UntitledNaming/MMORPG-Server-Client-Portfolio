#pragma once

#include "CoreMinimal.h"
#include "Character/M1BasePlayer.h"
#include "M1LocalPlayer.generated.h"

UCLASS()
class M1_API AM1LocalPlayer : public AM1BasePlayer
{
	GENERATED_BODY()
	
public:
	AM1LocalPlayer();

	virtual float GetMoveSpeed() override;
	virtual bool  GetMoveFlag()  override;

protected:
	// 카메라는 여기로
	UPROPERTY(VisibleAnywhere) TObjectPtr<class USpringArmComponent> SpringArmComponent;
	UPROPERTY(VisibleAnywhere) TObjectPtr<class UCameraComponent>    CameraComponent;

	UPROPERTY(VisibleAnywhere) bool bAutoAttack = false;
};

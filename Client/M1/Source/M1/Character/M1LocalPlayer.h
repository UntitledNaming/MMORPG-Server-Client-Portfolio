#pragma once

#include "CoreMinimal.h"
#include "Character/M1BasePlayer.h"
#include "M1LocalPlayer.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, int32, CurrentHealth, int32, MaxHealth);

UCLASS()
class M1_API AM1LocalPlayer : public AM1BasePlayer
{
	GENERATED_BODY()

public:
	AM1LocalPlayer();

	virtual void  ApplySpawnData(const FM1SpawnData& Data) override;
	virtual float GetMoveSpeed() override;
	virtual bool  GetMoveFlag()  override;
	virtual void  SetHP(int32 NewHP) override;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChanged OnHealthChanged;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
    
private:
	void  TickManaRegen(float DeltaTime);
	void  TickHPRegen(float DeltaTime);  

protected:
	// 카메라는 여기로
	UPROPERTY(VisibleAnywhere) TObjectPtr<class USpringArmComponent> SpringArmComponent;
	UPROPERTY(VisibleAnywhere) TObjectPtr<class UCameraComponent>    CameraComponent;

private:
	float ManaRegenAccum = 0.f;
	float HPRegenAccum = 0.f;   
};

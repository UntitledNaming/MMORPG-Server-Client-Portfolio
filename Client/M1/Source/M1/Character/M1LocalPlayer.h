#pragma once

#include "CoreMinimal.h"
#include "Character/M1BasePlayer.h"
#include "M1LocalPlayer.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, int32, CurrentHealth, int32, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnManaChanged, int32, CurrentMana, int32, MaxMana);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExpChanged, float, CurrentExp, float, RequiredExp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUp, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnStatChanged, int32, ATK, int32, DEF, int32, MaxHP, int32, MaxMP, int32, HPRegen, int32, MPRegen);

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
	virtual void  SetCurrentMana(uint16 Mana) override;
	virtual void  SetCurrentExp(uint32 NewExp) override;
	virtual void  OnLevelUpData(uint16 NewLevel, uint32 NewCurrentExp) override;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnManaChanged OnManaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnExpChanged OnExpChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLevelUp OnLevelUpdate;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStatChanged OnStatChanged;

protected:
	virtual void Tick(float DeltaTime) override;

	virtual void OnFinalStatChanged(const FM1CharacterStat& FinalStat) override;

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

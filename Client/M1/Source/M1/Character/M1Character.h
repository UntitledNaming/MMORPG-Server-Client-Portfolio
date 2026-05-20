#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "M1/System/Type/M1Type.h"
#include "Ability\M1PlayerActorComponent.h"
#include "M1Character.generated.h"

struct FInputActionValue;
class UWidgetComponent;
class UM1OverheadStatusWidget;

UCLASS()
class M1_API AM1Character : public ACharacter
{
    GENERATED_BODY()

public:
    AM1Character();

    FORCEINLINE bool IsDead()
    {
        if (HP <= 0)
            return true;

        return false;
    }
    FORCEINLINE void SetSpawnFlag(bool InFlag) { bSpawnFlag = InFlag; }
    FORCEINLINE bool GetSpawnFlag() { return bSpawnFlag; }

    // Getter
    FORCEINLINE float  GetMoveDirection() const { return MoveDirectionAngle; }
    FORCEINLINE float  GetCurrentHealth() const { return HP; }
    FORCEINLINE float  GetMaxHealth() const { return MaxHP; }
    FORCEINLINE float  GetHealthPercent() const { return MaxHP > 0 ? HP / MaxHP : 0.0f; }
    FORCEINLINE float  GetHitDirectionAngle() const { return HitDirectionAngle; }
    FORCEINLINE bool   IsHit()                const { return bIsHit; }
    FORCEINLINE bool   GetUseUpperBodyWhenMovingFlag()      const { return bUseUpperBodyWhenMoving; }
    FORCEINLINE UM1PlayerActorComponent* GetAbilityComponent() const { return AbilityComponent; }

    FORCEINLINE void SetUseUpperBodyWhenMovingFlag(bool InUseUpperBodyWhenMoving) { bUseUpperBodyWhenMoving = InUseUpperBodyWhenMoving; }

    virtual void  ApplySpawnData(const FM1SpawnData& Data);
    virtual float GetMoveSpeed() { return 0.f; }
    virtual bool  GetMoveFlag() { return false; }
    virtual void SetHP(int32 NewHP);

    // 피격 방향 각도로 피격 리액션 트리거 (0=정면, ±90=측면, ±180=후면)
    void TriggerHitReact(float HitAngle);

    // 피격 애니 종료 시 AnimBP Notify에서 호출
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ResetHitReact();

protected:
    virtual void BeginPlay() override;
    virtual void UpdateMoveDirection();

    FTimerHandle HitReactTimerHandle;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float HitReactDuration = 0.5f;

protected:

    UPROPERTY(VisibleAnywhere)
    uint64 EntityID = -1;

    UPROPERTY()
    uint16 HP = 100;

    UPROPERTY()
    uint16 MaxHP = 100;

    UPROPERTY(VisibleAnywhere)
    bool bSpawnFlag = false;

    // 이동 방향각 (-180~180): 0=앞, 90=우, -90=좌, ±180=뒤 — BS Direction축 입력값(공격시 재생할 애니 BS용 변수)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Movement")
    float MoveDirectionAngle = 0.f;

    // 피격 방향 각도 (-180 ~ 180): BS X축 입력값
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Combat")
    float HitDirectionAngle = 0.f;

    // 피격 상태 플래그: AnimBP 상태머신 전이 조건
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsHit = false;

    // 이동 중 상체 사용 플래그
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Combat")
    bool bUseUpperBodyWhenMoving = false;

protected:
    // PDA에 등록된 UM1CharacterAbilityDataAsset의 AssetName (서브클래스 BP에서 설정)
    UPROPERTY(EditDefaultsOnly, Category = "Ability")
    FName AbilityDataAssetName;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability")
    UM1PlayerActorComponent*      AbilityComponent;

    //////////////////////////////////////////////////////////////////////////////////
    // UI 처리
    //////////////////////////////////////////////////////////////////////////////////
public:
    void InitOverheadStatus(const FString& InName, int32 InCurrentHP, int32 InMaxHP);
    void SetOverheadHP(int32 InCurrentHP, int32 InMaxHP);
    void SetOverheadVisible(bool bVisible);
    UM1OverheadStatusWidget* GetOverheadStatusWidget();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
    UWidgetComponent* OverheadStatusComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UM1OverheadStatusWidget* OverheadStatusWidget = nullptr;

};

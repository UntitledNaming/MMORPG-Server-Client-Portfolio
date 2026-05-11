// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "M1AbilityTypes.h"
#include "M1AbilityBase.generated.h"

class AM1Character;
class UM1NetworkManager;

UCLASS(Blueprintable, Abstract)
class M1_API UM1AbilityBase : public UObject
{
	GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly)
    FAbilityFXData FXData;

    EAbilitySlot Slot = EAbilitySlot::BasicAttack;

    // 스킬 실행할 때(즉시 실행이면 재생까지 바로, 서버 검증형 스킬이면 패킷만 보내고 응답 받고 나서 재생하기)
    virtual void OnActivate (AM1Character* Owner) {}

    // 서버로부터 응답 왔을 때
    virtual void OnServerConfirmed(AM1Character* Owner) {}

    // 서버가 거절했을 때 (쿨다운, 조건 불만족 등)
    virtual void OnServerRejected(AM1Character* Owner) {}

    // 스킬 실행 끝냈을 때
    virtual void OnDeactivate(AM1Character* Owner) {}

protected:
    void PlayCastFX(AM1Character* Owner);
    void PlayHitFX(AM1Character* Target);
    UM1NetworkManager* GetNetworkManager(AM1Character* Owner) const;

};

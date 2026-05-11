// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ability/M1AbilityBase.h"
#include "M1Ability_ServerValidated.generated.h"

// 서버 검증형 스킬 공통 기반 (Skill1~4)
// OnActivate: 서버에 요청 패킷 전송 → 응답 대기
// OnServerConfirmed: 서버 승인 시 FX/애니 재생
// OnServerRejected: 거절 시 (쿨타임 등) 처리
UCLASS(Blueprintable, Abstract)
class M1_API UM1Ability_ServerValidated : public UM1AbilityBase
{
    GENERATED_BODY()

public:
    virtual void OnActivate(AM1Character* Owner) override;
    virtual void OnServerConfirmed(AM1Character* Owner) override;
    virtual void OnServerRejected(AM1Character* Owner) override;
    virtual void OnDeactivate(AM1Character* Owner) override;

private:
    // 서버로 패킷 보내고 받을 때 까지 중복 패킷 막는 플래그
    bool bIsPending = false;
};

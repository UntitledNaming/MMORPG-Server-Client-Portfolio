#pragma once

#include "CoreMinimal.h"
#include "Ability/M1AbilityBase.h"
#include "M1Ability_GroundSmash.generated.h"

UCLASS()
class M1_API UM1Ability_GroundSmash : public UM1AbilityBase
{
	GENERATED_BODY()
public:
	UM1Ability_GroundSmash();

	virtual void OnActivate(AM1Character* Owner) override;

	// 서버로부터 응답 왔을 때
	virtual void OnServerConfirmed(AM1Character* Owner) override;

private:
	AM1Character* CachedOwner = nullptr;
};

#pragma once

#include "CoreMinimal.h"
#include "M1Character.h"
#include "M1BasePlayer.generated.h"

UCLASS()
class M1_API AM1BasePlayer : public AM1Character
{
	GENERATED_BODY()
	
public:
	virtual void ApplySpawnData(const FM1SpawnData& Data) override;

	FORCEINLINE uint8 GetActionType() { return ActionType; }
	FORCEINLINE uint8 GetMoveMode() { return MoveMode; }

protected:
	UPROPERTY(VisibleAnywhere) FString NickName;
	UPROPERTY(VisibleAnywhere) int32   MP = 0;
	UPROPERTY(VisibleAnywhere) int32   MaxMP = 0;
	UPROPERTY(VisibleAnywhere) uint8   MoveMode = 0;
	UPROPERTY(VisibleAnywhere) uint8   ActionType = 0;
	UPROPERTY(VisibleAnywhere) bool    bMoving = false;
};

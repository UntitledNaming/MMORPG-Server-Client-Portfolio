#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "M1/System/Type/M1Type.h"
#include "M1Character.generated.h"

struct FInputActionValue;

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

    virtual void  ApplySpawnData(const FM1SpawnData& Data);
    virtual float GetMoveSpeed() { return 0.f; }
    virtual bool  GetMoveFlag() { return false; }

protected:

    UPROPERTY(VisibleAnywhere)
    uint64 EntityID = 0;

    UPROPERTY(VisibleAnywhere)
    int32 HP = 100;

    UPROPERTY(VisibleAnywhere)
    int32 MaxHP = 100;

    UPROPERTY(VisibleAnywhere)
    bool bSpawnFlag = false;
};

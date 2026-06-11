// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ContentsEnum.h"
#include "M1FieldDropItem.generated.h"


UCLASS()
class M1_API AM1FieldDropItem : public AActor
{
	GENERATED_BODY()
	
protected:	
	AM1FieldDropItem();

    UPROPERTY(VisibleAnywhere)
    class UNiagaraComponent* NiagaraEffect = nullptr;

    uint64  DropID = 0;
    uint32  ItemID = 0;
    uint16  Count = 0;

public:
    void    Init(uint64 InDropID, uint32 InItemID, FVector InLocation, uint16 InCount);
    void    SetRarityEffect(ITEM_RARITY Rarity);
    uint64  GetDropID() const { return DropID; };

};

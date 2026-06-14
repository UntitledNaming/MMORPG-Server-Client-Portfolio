#include "Item/M1FieldDropItem.h"
#include "NiagaraComponent.h"
#include "Item/M1ItemTable.h"


AM1FieldDropItem::AM1FieldDropItem()
{
	PrimaryActorTick.bCanEverTick = false;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    NiagaraEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraEffect"));
    NiagaraEffect->SetupAttachment(RootComponent);
    NiagaraEffect->bAutoActivate = false;
}

void    AM1FieldDropItem::Init(uint64 InDropID, uint32 InItemID, FVector InLocation, uint16 InCount)
{
    DropID = InDropID;
    ItemID = InItemID;
    Count = InCount;
    SetActorLocation(InLocation);

    const ItemData* Data = FM1ItemTable::GetItemData(ItemID);
    if (Data)
        SetRarityEffect(Data->itemRarity);
}

void    AM1FieldDropItem::SetRarityEffect(ITEM_RARITY Rarity)
{
    if (!NiagaraEffect)
        return;

    FLinearColor Color;
    switch (Rarity)
    {
    case ITEM_RARITY::NORMAL:
        Color = FLinearColor(1.f, 1.f, 1.f, 1.f);
        break;

    case ITEM_RARITY::MAGIC:
        Color = FLinearColor(0.f, 0.4f, 1.f, 1.f);
        break;
    }

    NiagaraEffect->SetColorParameter(TEXT("User.Color"), Color);
    NiagaraEffect->Activate(true);
}


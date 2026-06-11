#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ContentsEnum.h"
#include "System\Type\M1Type.h"
#include "M1ItemTooltipWidget.generated.h"

class UTextBlock;

UCLASS()
class M1_API UM1ItemTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    void SetItemData(ITEM_ID InItemID, const TArray<FRandomStat>& InRandomStats);

private:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_ItemName = nullptr;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_ItemType = nullptr;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_Rarity = nullptr;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_BaseStat = nullptr;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_RandomStat = nullptr;
};

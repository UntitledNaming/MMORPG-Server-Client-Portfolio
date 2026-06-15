#include "UI/M1ItemTooltipWidget.h"
#include "Item/M1ItemTable.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UM1ItemTooltipWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    FVector2D MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld());
    SetPositionInViewport(MousePos + FVector2D(15.f, 15.f), false);
}

void UM1ItemTooltipWidget::SetItemData(ITEM_ID InItemID, const TArray<FRandomStat>& InRandomStats)
{
    // 아이템 테이블에서 아이템 데이터 정보 얻기
    const ItemData* Data = FM1ItemTable::GetItemData(InItemID);
    if (!Data)
        return;

    // 아이템 이름
    if (Text_ItemName)
        Text_ItemName->SetText(FText::FromString(FM1ItemTable::GetItemName(InItemID)));

    // 희귀도
    if (Text_Rarity)
    {
        FString RarityStr;
        FSlateColor RarityColor;

        // 레어리티에 따라 텍스트 저장
        switch (Data->itemRarity)
        {
        case ITEM_RARITY::NORMAL:
            RarityStr = TEXT("Normal");
            RarityColor = FSlateColor(FLinearColor::White);
            break;

        case ITEM_RARITY::MAGIC:
            RarityStr = TEXT("Magic");
            RarityColor = FSlateColor(FLinearColor(0.f, 0.4f, 1.f));
            break;
        }

        // 해당 레어리티 텍스트 및 색 설정
        Text_Rarity->SetText(FText::FromString(RarityStr));
        Text_Rarity->SetColorAndOpacity(RarityColor);

    }

    // 아이템 타입
    if (Text_ItemType)
    {
        FString TypeStr;
        switch (Data->itemType)
        {
        case ITEM_TYPE::EQUIPMENT:  
            TypeStr = TEXT("Equipment"); 
            break;

        case ITEM_TYPE::CONSUMABLE: 
            TypeStr = TEXT("Consumable"); 
            break;

        default:                    
            TypeStr = TEXT("None"); 
            break;
        }

        Text_ItemType->SetText(FText::FromString(TypeStr));
    }

    // 기본 스탯
    if (Text_BaseStat)
    {
        // 문자열에 스탯 값이 0 넘으면 각 타입에 맞춰서 특정 문자열 저장
        FString StatStr;
        if (Data->baseStat.atk > 0)   
            StatStr += FString::Printf(TEXT("ATK +%d\n"), Data->baseStat.atk);

        if (Data->baseStat.def > 0)   
            StatStr += FString::Printf(TEXT("DEF +%d\n"), Data->baseStat.def);

        if (Data->baseStat.maxHP > 0) 
            StatStr += FString::Printf(TEXT("HP +%d\n"), Data->baseStat.maxHP);

        if (Data->baseStat.maxMP > 0) 
            StatStr += FString::Printf(TEXT("MP +%d\n"), Data->baseStat.maxMP);

        // 해당 문자열로 텍스트 블럭 세팅
        Text_BaseStat->SetText(FText::FromString(StatStr));
    }

    // 랜덤 스탯
    if (Text_RandomStat)
    {
        FString RandomStr;
        for (const FRandomStat& Stat : InRandomStats)
        {
            switch (Stat.RandomStatType)
            {
            case RANDOM_STAT_TYPE::ATK:     
                RandomStr += FString::Printf(TEXT("ATK +%d\n"), Stat.RandomStatValue);
                break;

            case RANDOM_STAT_TYPE::DEF:     
                RandomStr += FString::Printf(TEXT("DEF +%d\n"), Stat.RandomStatValue);
                break;

            case RANDOM_STAT_TYPE::MAX_HP:  
                RandomStr += FString::Printf(TEXT("HP +%d\n"), Stat.RandomStatValue);
                break;
            case RANDOM_STAT_TYPE::MAX_MP:  
                RandomStr += FString::Printf(TEXT("MP +%d\n"), Stat.RandomStatValue);
                break;
            case RANDOM_STAT_TYPE::HP_REGEN: 
                RandomStr += FString::Printf(TEXT("HP Regen +%d\n"),
                Stat.RandomStatValue); 
                break;
            case RANDOM_STAT_TYPE::MP_REGEN: 
                RandomStr += FString::Printf(TEXT("MP Regen +%d\n"),
                Stat.RandomStatValue); 
                break;
            default: break;
            }
        }

        Text_RandomStat->SetText(FText::FromString(RandomStr));
    }
}
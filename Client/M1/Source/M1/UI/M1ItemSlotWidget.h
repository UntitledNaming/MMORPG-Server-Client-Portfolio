#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "ContentsEnum.h"
#include "M1ItemSlotWidget.generated.h"

class UM1ItemManager;
class UM1DraggedItemWidget;
class UM1ItemTooltipWidget;

UCLASS()
class M1_API UM1ItemSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	TObjectPtr<UImage> Image_Icon;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	TObjectPtr<UTextBlock> Text_Count;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	TObjectPtr<UImage> Image_CoolTime;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	TObjectPtr<UImage> Image_Background;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UM1DraggedItemWidget> DragWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UM1ItemTooltipWidget> TooltipWidgetClass;

	UPROPERTY()
	UM1ItemManager* ItemManager = nullptr;

	SLOT_TYPE SlotType = SLOT_TYPE::NONE;
	int16     SlotIndex = -1;

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry&, const FPointerEvent&) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry&, const FPointerEvent&) override;
	virtual void NativeOnMouseEnter(const FGeometry&, const FPointerEvent&) override;
	virtual void NativeOnMouseLeave(const FPointerEvent&) override;

	void InitSlot(SLOT_TYPE InSlotType, int16 InSlotIndex);
	void RefreshSlot();

	// 어떤 아이템ID에 대해서 어떤 이미지를 쓸 지는 에디터에서 처리해야 함. 
	// C++에서 호출하면 BP 에디터에서 실제로 정의하여 구현
	// 비어 있으면 빈 이미지로 설정하고 
	// 아이템이 있으면 ItemID로 아이템 아이콘 테이블 조회해서 해당 아이콘 텍스쳐 가져와서 설정
	UFUNCTION(BlueprintImplementableEvent)
	void OnSlotDataChanged(int32 ItemID, int32 Count, bool bIsEmpty);

private:
	void UpdateCooldownImage();

};

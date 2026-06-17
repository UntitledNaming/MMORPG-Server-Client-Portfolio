#include "UI/M1ItemSlotWidget.h"
#include "UI/M1DraggedItemWidget.h"
#include "System/M1ItemManager.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UM1ItemSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ItemManager = GetGameInstance()->GetSubsystem<UM1ItemManager>();
	if (Image_CoolTime)
	{
		Image_CoolTime->SetVisibility(ESlateVisibility::Collapsed);
		Image_CoolTime->SetColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.5f));
	}

	if (Image_Background)
		Image_Background->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UM1ItemSlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateCooldownImage();
}

FReply UM1ItemSlotWidget::NativeOnMouseButtonDown(const FGeometry& GeoMetry, const FPointerEvent& PointerEvent)
{
	// 우클릭이 아니면 기본 동작으로 처리
	if (PointerEvent.GetEffectingButton() != EKeys::RightMouseButton)
		return Super::NativeOnMouseButtonDown(GeoMetry, PointerEvent);

	// 아이템 매니저 없으면 리턴
	if (!ItemManager)
		return FReply::Handled();

	// 해당 슬롯 쿨타임 체크
	if (ItemManager->IsSlotOnCoolTime(static_cast<uint8>(SlotType), SlotIndex))
		return FReply::Handled();

	// 슬롯 타입에 해당하는 슬롯 데이터 가져오기
	const FItemSlotData* SlotData = nullptr;
	switch (SlotType)
	{
	case SLOT_TYPE::INVENTORY:  
		SlotData = &ItemManager->GetInventorySlot(SlotIndex); 
		break;

	case SLOT_TYPE::EQUIPMENT:  
		SlotData = &ItemManager->GetEquipmentSlot(SlotIndex); 
		break;

	case SLOT_TYPE::QUICKSLOT:  
		SlotData = &ItemManager->GetQuickSlot(SlotIndex);     
		break;
	}

	// 슬롯 데이터 자체가 없거나 빈 슬롯이면 리턴
	if (!SlotData || SlotData->ItemID == 0)
		return FReply::Handled();

	ItemManager->HideTooltip();

	// DragWidgetClass가 설정되어 있으면
	if (DragWidgetClass)
	{
		UM1DraggedItemWidget* DragWidget = CreateWidget<UM1DraggedItemWidget>(GetOwningPlayer(), DragWidgetClass);
		if (DragWidget)
		{
			DragWidget->AddToViewport(100);

			// 현재 슬롯 텍스처를 드래그 위젯에 복사
			if (Image_Icon)
			{
				UTexture2D* Texture = Cast<UTexture2D>(Image_Icon->GetBrush().GetResourceObject());
				DragWidget->SetIcon(Texture);

				// 소스 슬롯 이미지 숨기기
				Image_Icon->SetVisibility(ESlateVisibility::Hidden);
			}

			ItemManager->StartDrag(SlotType, SlotIndex, DragWidget);
			ItemManager->SetDragHover(SlotType, SlotIndex);
		}
	}

	return FReply::Handled();
}

FReply UM1ItemSlotWidget::NativeOnMouseButtonUp(const FGeometry& GeoMetry, const FPointerEvent& PointerEvent)
{
	if (PointerEvent.GetEffectingButton() != EKeys::RightMouseButton)
		return Super::NativeOnMouseButtonUp(GeoMetry, PointerEvent);

	// 아이템 매니저가 없거나 현재 드래그 중이 아니라면 리턴
	if (!ItemManager || !ItemManager->IsDragging())
		return FReply::Handled();

	// 드래그 시작 할때의 슬롯 타입 및 위치 정보 가져오기
	const SLOT_TYPE SrcType = ItemManager->GetDragStartSourceType();
	const int16     SrcIndex = ItemManager->GetDragStartSourceIndex();

	// 현재 마우스 위치에 대한 슬롯 타입 및 위치 정보 가져오기
	const SLOT_TYPE HoverType = ItemManager->GetDragHoverType();
	const int16     HoverIndex = ItemManager->GetDragHoverIndex();

	// 현재 마우스 위치에 있는 슬롯 타입이 None이면 밖으로 나간거니 Delete 메세지 전달
	if (HoverType == SLOT_TYPE::NONE)
	{
		ItemManager->TrySendDeleteItem(static_cast<uint8>(SrcType), SrcIndex);
	}

	// 현재 마우스 위치에 있는 슬롯 타입과 드래그 시작 위치 타입과 위치가 같으면 Use 패킷 전달(그 자리에서 우클릭 뗀것)
	else if (HoverType == SrcType && HoverIndex == SrcIndex)
	{
		// Use는 드래그가 아니므로 이미지 즉시 복원
		if (Image_Icon)
			Image_Icon->SetVisibility(ESlateVisibility::HitTestInvisible);
		ItemManager->TrySendUseItem(static_cast<uint8>(SrcType), SrcIndex);
	}

	// 그게 아니면 드래그 시작 위치와 현재 마우스 위치에 있는 슬롯 타입에 대한 Swap 패킷 전달
	else
	{
		ItemManager->TrySendSwapSlot(
			static_cast<uint8>(SrcType), SrcIndex,
			static_cast<uint8>(HoverType), HoverIndex);
	}

	// 드래그 끝
	ItemManager->EndDrag();
	return FReply::Handled();
}

void UM1ItemSlotWidget::NativeOnMouseEnter(const FGeometry& GeoMetry, const FPointerEvent& PointerEvent)
{
	Super::NativeOnMouseEnter(GeoMetry, PointerEvent);

	if (ItemManager->IsDragging())
	{
		// 드래그 중이면 hover 슬롯 갱신
		ItemManager->SetDragHover(SlotType, SlotIndex);
	}
	else
	{
		// 드래그 중 아니면 툴팁 표시
		ItemManager->ShowTooltip(SlotType, SlotIndex, TooltipWidgetClass);
	}
}

void UM1ItemSlotWidget::NativeOnMouseLeave(const FPointerEvent& PointerEvent)
{
	Super::NativeOnMouseLeave(PointerEvent);

	ItemManager->ClearDragHover();
	ItemManager->HideTooltip();
}

void UM1ItemSlotWidget::InitSlot(SLOT_TYPE InSlotType, int16 InSlotIndex)
{
	SlotType = InSlotType;
	SlotIndex = InSlotIndex;

	RefreshSlot();
}

void UM1ItemSlotWidget::RefreshSlot()
{
	if (!ItemManager)
		return;

	const FItemSlotData* SlotData = nullptr;

	// 현재 슬롯 타입에 따라서 슬롯 데이터 가져오기
	switch (SlotType)
	{
	case SLOT_TYPE::INVENTORY:
		SlotData = &ItemManager->GetInventorySlot(SlotIndex);
		break;

	case SLOT_TYPE::EQUIPMENT:
		SlotData = &ItemManager->GetEquipmentSlot(SlotIndex);
		break;

	case SLOT_TYPE::QUICKSLOT:
		SlotData = &ItemManager->GetQuickSlot(SlotIndex);
		break;

	default:
		return;
	}

	// 슬롯이 빈 상태인지 
	bool bIsEmpty = SlotData->ItemID == 0;

	// 텍스트 블럭이 있는지
	if (Text_Count)
	{
		// 슬롯 데이터가 있고 슬롯에 있는 아이템 갯수가 2개 이상이면 갯수 표시
		if (!bIsEmpty && SlotData->Count > 1)
		{
			Text_Count->SetText(FText::AsNumber(SlotData->Count));
			Text_Count->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		// 그게 아니면 감추기
		else
		{
			Text_Count->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	// 드래그로 숨겨진 이미지 복원 (BP가 visibility를 건드리지 않는 경우 대비)
	if (Image_Icon)
		Image_Icon->SetVisibility(ESlateVisibility::HitTestInvisible);

	// BP 이벤트
	OnSlotDataChanged(static_cast<int32>(SlotData->ItemID), static_cast<int32>(SlotData->Count), bIsEmpty);
}

void UM1ItemSlotWidget::UpdateCooldownImage()
{
	if (!ItemManager || !Image_CoolTime)
		return;

	if (SlotType == SLOT_TYPE::NONE || SlotIndex < 0)
	{
		Image_CoolTime->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const float Ratio = ItemManager->GetSlotCoolTimeRatio(
		static_cast<uint8>(SlotType),
		SlotIndex
	);

	if (Ratio > 0.f)
	{
		Image_CoolTime->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		Image_CoolTime->SetVisibility(ESlateVisibility::Collapsed);
	}
}
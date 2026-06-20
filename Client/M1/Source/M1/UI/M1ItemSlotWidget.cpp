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

	if (ItemManager)
		ItemManager->RegisterSlotWidget(this);
}

void UM1ItemSlotWidget::NativeDestruct()
{
	if (ItemManager)
		ItemManager->UnregisterSlotWidget(this);

	Super::NativeDestruct();
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

	// 슬롯 데이터 자체가 없거나 빈 슬롯이면 리턴 (빈 슬롯에서 스왑 시작 차단 포함)
	if (!SlotData || SlotData->ItemID == 0)
		return FReply::Handled();

	if (!DragWidgetClass)
		return FReply::Handled();

	ItemManager->HideTooltip();

	UM1DraggedItemWidget* DragWidget = CreateWidget<UM1DraggedItemWidget>(GetOwningPlayer(), DragWidgetClass);
	if (!DragWidget)
		return FReply::Handled();

	DragWidget->AddToViewport(100);

	// 현재 슬롯 텍스처를 드래그 위젯에 복사 + 소스 슬롯 이미지 숨기기
	if (Image_Icon)
	{
		UTexture2D* Texture = Cast<UTexture2D>(Image_Icon->GetBrush().GetResourceObject());
		DragWidget->SetIcon(Texture);
		Image_Icon->SetVisibility(ESlateVisibility::Hidden);
	}

	ItemManager->StartDrag(SlotType, SlotIndex, DragWidget);

	// 마우스 캡처 → 화면 어디서 떼든 Up이 이 슬롯으로 돌아옴 (슬롯 밖에서 떼는 Delete 정상 처리)
	return FReply::Handled().CaptureMouse(TakeWidget());
}

FReply UM1ItemSlotWidget::NativeOnMouseButtonUp(const FGeometry& GeoMetry, const FPointerEvent& PointerEvent)
{
	if (PointerEvent.GetEffectingButton() != EKeys::RightMouseButton)
		return Super::NativeOnMouseButtonUp(GeoMetry, PointerEvent);

	// 아이템 매니저가 없거나 현재 드래그 중이 아니라면 캡처만 해제하고 리턴
	if (!ItemManager || !ItemManager->IsDragging())
		return FReply::Handled().ReleaseMouseCapture();

	// 드래그 시작 할때의 슬롯 타입 및 위치 정보 가져오기
	const SLOT_TYPE SrcType  = ItemManager->GetDragStartSourceType();
	const int16     SrcIndex = ItemManager->GetDragStartSourceIndex();

	// 뗀 순간 커서 좌표로 어떤 슬롯 위에 있는지 히트테스트 (캡처 중엔 Enter/Leave가 안 오므로 좌표로 판정)
	const FVector2D UpPos = PointerEvent.GetScreenSpacePosition();

	SLOT_TYPE TgtType  = SLOT_TYPE::NONE;
	int16     TgtIndex = -1;
	const bool bOverSlot = ItemManager->FindSlotUnderCursor(UpPos, TgtType, TgtIndex);

	// 캡처 덕에 Up은 항상 소스 슬롯(this)으로 돌아온다.
	// 빠른 클릭이면 커서가 소스 슬롯을 벗어나지 않는데, FindSlotUnderCursor는
	// GetCachedGeometry()(직전 페인트 기준)를 써서 가끔 히트 실패 → Delete로 빠진다.
	// 이벤트로 들어온 '신선한' GeoMetry로 소스 슬롯 위인지 먼저 확정해 오삭제를 막는다.
	const bool bOverSource = GeoMetry.IsUnderLocation(UpPos);

	// 드래그 종료 (숨긴 소스 아이콘은 EndDrag의 Broadcast로 즉시 복원)
	ItemManager->EndDrag();

	if (bOverSource)
	{
		// 커서가 소스 슬롯 위 → 무조건 사용 (빠른 클릭 오삭제 방지)
		ItemManager->TrySendUseItem(static_cast<uint8>(SrcType), SrcIndex);
	}
	else if (!bOverSlot)
	{
		// 슬롯/패널 밖에서 뗌 → 삭제
		ItemManager->TrySendDeleteItem(static_cast<uint8>(SrcType), SrcIndex);
	}
	else if (TgtType == SrcType && TgtIndex == SrcIndex)
	{
		// 같은 슬롯에서 뗌 → 사용
		ItemManager->TrySendUseItem(static_cast<uint8>(SrcType), SrcIndex);
	}
	else
	{
		// 다른 슬롯 → 스왑 (유효 조합 검증은 ItemManager가 처리)
		ItemManager->TrySendSwapSlot(
			static_cast<uint8>(SrcType), SrcIndex,
			static_cast<uint8>(TgtType), TgtIndex);
	}

	return FReply::Handled().ReleaseMouseCapture();
}

void UM1ItemSlotWidget::NativeOnMouseEnter(const FGeometry& GeoMetry, const FPointerEvent& PointerEvent)
{
	Super::NativeOnMouseEnter(GeoMetry, PointerEvent);

	// 드래그 중엔 마우스가 캡처되어 다른 슬롯 Enter가 안 옴 → 비드래그일 때만 툴팁 표시
	if (ItemManager && !ItemManager->IsDragging())
		ItemManager->ShowTooltip(SlotType, SlotIndex, TooltipWidgetClass);
}

void UM1ItemSlotWidget::NativeOnMouseLeave(const FPointerEvent& PointerEvent)
{
	Super::NativeOnMouseLeave(PointerEvent);

	if (ItemManager)
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
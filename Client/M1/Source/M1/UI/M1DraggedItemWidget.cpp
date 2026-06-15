#include "UI/M1DraggedItemWidget.h"
#include "Components\Image.h"
#include "Blueprint\WidgetLayoutLibrary.h"

void UM1DraggedItemWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 마우스가 움직이면 마우스 위치에서 히트 테스트를 하여 가장 가까이 있는 UserWidget 객체에게 이벤트를 보냄.
    // 이 객체가 ItemSlotWidget 객체보다 더 가까이 있으면 UE가 이 클래스에게 Enter 이벤트를 보냄.
    // 따라서 하위에 있는 ItemSlotWidget에게 이벤트가 가지 않음. 
    // 그래서 HitTestInvisible을 전달하여 히트 테스트할때 무시하라고 함.
    SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UM1DraggedItemWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // 매 프레임 현재 마우스 위치 가져와서 위젯의 위치를 거기로 이동. 아이콘이 마우스 따라다는 처리
    FVector2D MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld());
    SetPositionInViewport(MousePos, false);
}

void UM1DraggedItemWidget::SetIcon(UTexture2D* Texture)
{
    if (!Image_Icon)
        return;

    // 드래그 시작할때 ItemSlotWidget에서 이 함수 호출해서 아이콘 이미지를 세팅함.
    Image_Icon->SetBrushFromTexture(Texture, false);
    Image_Icon->SetDesiredSizeOverride(FVector2D(48.f, 48.f));
}
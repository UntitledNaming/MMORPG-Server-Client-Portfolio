#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "M1DraggedItemWidget.generated.h"

UCLASS()
class M1_API UM1DraggedItemWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta = (BindWidget))
	class UImage* Image_Icon = nullptr;

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry&, float) override;

public:
	        void SetIcon(UTexture2D* Texture);
};

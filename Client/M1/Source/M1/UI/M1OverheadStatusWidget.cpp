#include "UI/M1OverheadStatusWidget.h"
#include "Components/TextBlock.h"
#include "Components/SizeBox.h"


void UM1OverheadStatusWidget::InitOverheadStatus(const FText& InName, int32 InCurrentHP, int32 InMaxHP)
{
	SetOverheadName(InName);
	SetOverheadHP(InCurrentHP, InMaxHP);
}

void UM1OverheadStatusWidget::SetOverheadName(const FText& InName)
{
	if (Text_Name == nullptr)
		return;

	Text_Name->SetText(InName);
}

void UM1OverheadStatusWidget::SetOverheadHP(int32 InCurrentHP, int32 InMaxHP)
{
	if (SizeBox_HPClip == nullptr)
		return;

	float Percent = 0.f;

	if (InMaxHP > 0.f)
	{
		Percent = (float)InCurrentHP / (float)InMaxHP;
	}

	Percent = FMath::Clamp(Percent, 0.f, 1.f);

	// 너무 작으면 아예 0으로 처리
	if (Percent <= 0.02f)
	{
		Percent = 0.0f;
	}

	const float NewWidth = MaxBarWidth * Percent;
	SizeBox_HPClip->SetWidthOverride(NewWidth);
}
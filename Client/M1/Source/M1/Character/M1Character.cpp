#include "M1Character.h"
#include "ContentsDefine.h"
#include "Ability\M1PlayerActorComponent.h"
#include "UI\M1OverheadStatusWidget.h"
#include "Data\M1CharacterAbilityDataAsset.h"
#include "System\M1AssetManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Components\CapsuleComponent.h"

AM1Character::AM1Character()
{
	PrimaryActorTick.bCanEverTick = false;
	AbilityComponent = CreateDefaultSubobject<UM1PlayerActorComponent>(TEXT("AbilityComponent"));

	OverheadStatusComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadStatusComponent"));
	OverheadStatusComponent->SetupAttachment(GetRootComponent());

	OverheadStatusComponent->SetVisibility(false);
	OverheadStatusComponent->SetWidgetSpace(EWidgetSpace::Screen);
	OverheadStatusComponent->SetDrawSize(FVector2D(200.f, 50.f));
	OverheadStatusComponent->SetPivot(FVector2D(0.5f, 1.f));

}

void AM1Character::ApplySpawnData(const FM1SpawnData& Data)
{
	// 매개인자로 받은 구조체로 초기화
	EntityID = Data.EntityID;
	SetActorLocation(Data.Location);
	SetActorRotation(Data.Rotation);
	HP = Data.HP;
	MaxHP = Data.MaxHP;
}

void AM1Character::SetHP(int32 NewHP)
{
    HP = FMath::Clamp(NewHP, 0, MaxHP);
}

void AM1Character::TriggerHitReact(float HitAngle)
{
    HitDirectionAngle = HitAngle;
    bIsHit            = true;
	bUseUpperBodyWhenMoving = true;

    GetWorldTimerManager().ClearTimer(HitReactTimerHandle);
    GetWorldTimerManager().SetTimer(HitReactTimerHandle, this, &AM1Character::ResetHitReact, HitReactDuration, false);
}

void AM1Character::ResetHitReact()
{
    bIsHit = false;
	bUseUpperBodyWhenMoving = false;
}

void AM1Character::UpdateMoveDirection()
{
    FVector Vel = GetVelocity();
    if (Vel.SizeSquared2D() < 1.f)
    {
        MoveDirectionAngle = 0.f;
        return;
    }
    FVector VelNorm = Vel.GetSafeNormal2D();
    FVector Fwd     = GetActorForwardVector().GetSafeNormal2D();
    FVector Right   = GetActorRightVector().GetSafeNormal2D();
    MoveDirectionAngle = FMath::RadiansToDegrees(
        FMath::Atan2(FVector::DotProduct(Right, VelNorm),
                     FVector::DotProduct(Fwd,   VelNorm)));
}

void AM1Character::BeginPlay()
{
	Super::BeginPlay();

	if (!AbilityDataAssetName.IsNone())
	{
		UM1CharacterAbilityDataAsset* AbilityData =
			UM1AssetManager::GetAssetByName<UM1CharacterAbilityDataAsset>(AbilityDataAssetName);
		if (AbilityData)
			AbilityComponent->InitializeFromData(AbilityData);
	}

}

void AM1Character::InitOverheadStatus(const FString& InName, int32 InCurrentHP, int32 InMaxHP)
{
	OverheadStatusWidget = GetOverheadStatusWidget();

	if (OverheadStatusWidget == nullptr && OverheadStatusComponent)
	{
		OverheadStatusWidget =
			Cast<UM1OverheadStatusWidget>(OverheadStatusComponent->GetUserWidgetObject());
	}

	if (OverheadStatusWidget == nullptr)
		return;

	OverheadStatusWidget->InitOverheadStatus(FText::FromString(InName), InCurrentHP, InMaxHP);
}

void AM1Character::SetOverheadHP(int32 InCurrentHP, int32 InMaxHP)
{
	if (OverheadStatusWidget == nullptr && OverheadStatusComponent)
	{
		OverheadStatusWidget =
			Cast<UM1OverheadStatusWidget>(OverheadStatusComponent->GetUserWidgetObject());
	}

	if (OverheadStatusWidget == nullptr)
		return;

	OverheadStatusWidget->SetOverheadHP(InCurrentHP, InMaxHP);
}

void AM1Character::SetOverheadVisible(bool bVisible)
{
	if (OverheadStatusComponent == nullptr)
		return;

	OverheadStatusComponent->SetVisibility(bVisible);
}

UM1OverheadStatusWidget* AM1Character::GetOverheadStatusWidget()
{
	if (OverheadStatusWidget)
		return OverheadStatusWidget;

	if (OverheadStatusComponent == nullptr)
		return nullptr;

	OverheadStatusComponent->InitWidget();

	OverheadStatusWidget =
		Cast<UM1OverheadStatusWidget>(
			OverheadStatusComponent->GetUserWidgetObject()
		);

	return OverheadStatusWidget;
}
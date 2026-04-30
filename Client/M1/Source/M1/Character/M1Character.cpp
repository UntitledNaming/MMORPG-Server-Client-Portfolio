#include "M1Character.h"
#include "ContentsDefine.h"
#include "Components/WidgetComponent.h"

AM1Character::AM1Character()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AM1Character::BeginPlay()
{
	Super::BeginPlay();
	
}

void AM1Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsMyPlayer)
		return;

	FVector NewLocation = GetActorLocation();

	if (bServerMoveFlag)
	{
		const float Rad = FMath::DegreesToRadians(GetActorRotation().Yaw);
		FVector MoveDir(FMath::Cos(Rad), FMath::Sin(Rad), 0.f);

		NewLocation += MoveDir * MoveSpeed * DeltaTime;
		bRenderMoveFlag = true;
	}

	// 서버는 멈췄는데 수렴 플래그가 켜져있으면
	else if (bNeedStopCorrection)
	{
		NewLocation = FMath::VInterpConstantTo(
			NewLocation,
			StopTargetLocation,
			DeltaTime,
			MoveSpeed);

		// 수렴할 위치와 정지 위치 차이가 2cm 아래로 갔으면 이제 정지 위치를 NewLocation으로 해서 위치 순간이동
		// 수렴 플래그 끄고 렌더링 플래그도 끄기
		if (FVector::Dist2D(NewLocation, StopTargetLocation) < 2.f)
		{
			NewLocation = StopTargetLocation;
			bNeedStopCorrection = false;
			bRenderMoveFlag = false;
		}
	}

	// 서버도 멈추고 수렴 플래그도 꺼져 있으면 렌더링 플래그 끄기
	else
	{
		bRenderMoveFlag = false;
	}

}

void AM1Character::ApplySpawnData(const FM1SpawnData& Data)
{
	// 매개인자로 받은 구조체로 초기화
	EntityID = Data.EntityID;
	SetActorLocation(Data.Location);
	SetActorRotation(Data.Rotation);
	ActionType = static_cast<uint8>(Data.ActionType);
	HP = Data.HP;
	MaxHP = Data.MaxHP;
	bServerMoveFlag = Data.MoveFlag;
	bRenderMoveFlag = Data.MoveFlag;

	if (Data.MoveFlag)
	{
		MoveSpeed = UserConst::WALK_SPEED;
	}
	else
	{
		MoveSpeed = 0;
	}
}


#include "M1Character.h"
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

}


void AM1Character::Destroy()
{

}

void AM1Character::ApplySpawnData(const FM1SpawnData& Data)
{
	EntityID = Data.EntityID;
	SetActorLocation(Data.Location);
	SetActorRotation(Data.Rotation);
	ActionType = Data.ActionType;
	InputMask = Data.InputMask;
	HP = Data.HP;
	MaxHP = Data.MaxHP;
}

void AM1Character::ApplyStateData(EM1ActionType NewAction, uint8 NewInputMask)
{
	ActionType = NewAction;
	InputMask = NewInputMask;
}

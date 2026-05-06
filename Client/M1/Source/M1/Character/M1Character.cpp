#include "M1Character.h"
#include "ContentsDefine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"

AM1Character::AM1Character()
{

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


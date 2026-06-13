// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/M1LocalPlayer.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework//SpringArmComponent.h"
#include "Camera//CameraComponent.h"

AM1LocalPlayer::AM1LocalPlayer()
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;


	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 600.0f;
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;

	PrimaryActorTick.bCanEverTick = true;
}

void AM1LocalPlayer::ApplySpawnData(const FM1SpawnData& Data)
{
	Super::ApplySpawnData(Data);
	SetLevelStat(Level);                              // Mana, Stat 브로드 캐스트

	OnExpChanged.Broadcast(CurrentExp, RequiredExp);  // HUD EXP바 초기값
	OnHealthChanged.Broadcast(HP, MaxHP);             // HUD HP바 초기값
}

void AM1LocalPlayer::BeginPlay()
{
	Super::BeginPlay();
	OnHealthChanged.Broadcast(HP, MaxHP);
}

void AM1LocalPlayer::SetHP(int32 NewHP)
{
	Super::SetHP(NewHP);
	OnHealthChanged.Broadcast(HP, MaxHP);

	if (HP <= 0)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
			PC->DisableInput(PC);
	}
}

void AM1LocalPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateMoveDirection();
	TickManaRegen(DeltaTime);
	TickHPRegen(DeltaTime);
}

float AM1LocalPlayer::GetMoveSpeed()
{
	return GetCharacterMovement()->Velocity.Size2D();
}
bool AM1LocalPlayer::GetMoveFlag()
{
	return GetCharacterMovement()->Velocity.Size2D() > 1.f;
}

void AM1LocalPlayer::TickManaRegen(float DeltaTime)
{
	// 마나 다 찼으면 pass
	if (MP >= MaxMP)
		return;

	ManaRegenAccum += DeltaTime;

	// 1초 안지났으면 PASS
	if (ManaRegenAccum < (float)UserConst::USER_MP_REGEN_TIME)
		return;

	// 지났으면 다시 합산 초기화
	ManaRegenAccum -= (float)UserConst::USER_MP_REGEN_TIME;

	MP += MPRegenPerSec;

	if (MP > MaxMP)
		MP = MaxMP;

	OnManaChanged.Broadcast(MP, MaxMP);

}

void  AM1LocalPlayer::TickHPRegen(float DeltaTime)
{
	if (HP >= MaxHP) 
		return;

	HPRegenAccum += DeltaTime;
	if (HPRegenAccum < (float)UserConst::USER_HP_REGEN_TIME)
		return;

	HPRegenAccum -= (float)UserConst::USER_HP_REGEN_TIME;

	SetHP(HP + HPRegenPerSec);  // SetHP 내부에서 Clamp + OnHealthChanged.Broadcast 처리
}
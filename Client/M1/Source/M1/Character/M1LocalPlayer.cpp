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
	ManaRegenAccum += DeltaTime;

	if (ManaRegenAccum < 1.f) 
		return;

	ManaRegenAccum -= 1.f;

	MP += MPRegenPerSec;
	if (MP > MaxMP)
		MP = MaxMP;

	OnManaChanged.Broadcast(MP, MaxMP);

}
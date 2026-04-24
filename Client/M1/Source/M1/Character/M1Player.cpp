// Fill out your copyright notice in the Description page of Project Settings.


#include "M1Player.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework//SpringArmComponent.h"
#include "Camera//CameraComponent.h"

AM1Player::AM1Player()
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
	SpringArmComponent->TargetArmLength = 500.0f;
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;

}

void AM1Player::BeginPlay()
{
	Super::BeginPlay();

}


void AM1Player::ApplySpawnData(const FM1SpawnData& Data)
{
	Super::ApplySpawnData(Data);

	if (bIsMyPlayer)
	{
		MP = Data.MP;
		MaxMP = Data.MaxMP;
	}

	bIsSpawnInit = true;
}


void AM1Player::UpdateAutoAttack()
{
	if (!bAutoAttack)
		return;

}


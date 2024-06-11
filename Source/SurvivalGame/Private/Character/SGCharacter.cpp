// Copyright Marco Freemantle

#include "Character/SGCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

ASGCharacter::ASGCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 250.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void ASGCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASGCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);
}

void ASGCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ASGCharacter::AimOffset(float DeltaTime)
{
	// Should only return if no weapon is equipped
	return;
	// FVector Velocity = GetVelocity();
	// Velocity.Z = 0.f;
	// float Speed = Velocity.Size();
	// bool bIsInAir = GetCharacterMovement()->IsFalling();
	//
	// if(Speed == 0.f && !bIsInAir) // Standing still & not jumping
	// {
	// 	FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
	// 	FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
	// 	AO_Yaw = DeltaAimRotation.Yaw;
	// 	if(TurningInPlace == ETurningInPlace::ETIP_NotTurning)
	// 	{
	// 		InterpAO_Yaw = AO_Yaw;
	// 	}
	// 	bUseControllerRotationYaw = true;
	// 	TurnInPlace(DeltaTime);
	// }
	// if(Speed > 0.f || bIsInAir) // Running or Jumping
	// {
	// 	StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
	// 	AO_Yaw = 0.f;
	// 	bUseControllerRotationYaw = true;
	// 	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	// }
	//
	// AO_Pitch = GetBaseAimRotation().Pitch;
	// if(AO_Pitch > 90.f && !IsLocallyControlled())
	// {
	// 	// Map pitch from [270, 360) to [-90, 0)
	// 	FVector2D InRange(270.f, 360.f);
	// 	FVector2D OutRange(-90.f, 0.f);
	// 	AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	// }
}

void ASGCharacter::TurnInPlace(float DeltaTime)
{
	if(AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if(AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if(TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 8.f);
		AO_Yaw = InterpAO_Yaw;
		if(FMath::Abs(AO_Yaw) < 1.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}


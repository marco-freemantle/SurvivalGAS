// Copyright Marco Freemantle

#include "Character/SGCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Interfaces/LockonInterface.h"
#include "Kismet/KismetMathLibrary.h"

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
	if(bShouldRotate && LockonTarget)
	{
		const FRotator PlayerRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(),LockonTarget->GetActorLocation());
		GetController()->SetControlRotation(PlayerRot);
	}
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

void ASGCharacter::EngageLockon()
{
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(750.0f);
	TArray<FHitResult> OutResults;
	GetWorld()->SweepMultiByChannel(OutResults, GetActorLocation(), GetActorLocation(), FQuat::Identity, ECC_Pawn, SphereShape);

	for (auto Hit : OutResults)
	{
		if(Hit.GetActor()->Implements<ULockonInterface>())
		{
			LockonTargets.Add(Hit.GetActor());
		}
	}

	float Distance = 1000000.f;

	for (const auto Target : LockonTargets)
	{
		if(FVector::Dist(GetActorLocation(), Target->GetActorLocation()) <= Distance)
		{
			Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
			LockonTarget = Target;
		}
	}

	if(LockonTarget)
	{
		bIsLockedOnTarget = true;
		bShouldRotate = true;
		GetController()->SetIgnoreLookInput(true);
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = true;

		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(BreakLockonTimer, this, &ASGCharacter::CheckLockonDistance, 0.15f, true);
		}
	}
}

void ASGCharacter::DisenganeLockon()
{
	LockonTargets = TArray<AActor*>();
	LockonTarget = nullptr;

	bIsLockedOnTarget = false;
	bShouldRotate = false;
	GetController()->ResetIgnoreLookInput();
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
}

void ASGCharacter::CheckLockonDistance()
{
	if(LockonTarget)
	{
		if(FVector::Dist(GetActorLocation(), LockonTarget->GetActorLocation()) > 1000)
		{
			DisenganeLockon();
			BreakLockonTimer.Invalidate();
		}
	}
}


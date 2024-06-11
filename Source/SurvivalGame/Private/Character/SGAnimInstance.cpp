// Copyright Marco Freemantle

#include "Character/SGAnimInstance.h"
#include "Character/SGCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Animation blueprint BeginPlay equivalent
void USGAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	SGCharacter = Cast<ASGCharacter>(TryGetPawnOwner());
}

// Animation blueprint Tick equivalent
void USGAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if(SGCharacter == nullptr)
	{
		SGCharacter = Cast<ASGCharacter>(TryGetPawnOwner());
	}
	if(SGCharacter == nullptr) return;

	FVector Velocity = SGCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = SGCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = SGCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;
	bIsCrouched = SGCharacter->bIsCrouched;

	// Offset Yaw for strafing
	FRotator AimRotation = SGCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(SGCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaSeconds, 10.f);
	YawOffset = DeltaRotation.Yaw;

	// Aim offsets
	AO_Yaw = SGCharacter->GetAO_Yaw();
	AO_Pitch = SGCharacter->GetAO_Pitch();
}
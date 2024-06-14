// Copyright Marco Freemantle

#include "Character/SGAnimInstance.h"
#include "KismetAnimationLibrary.h"
#include "Character/SGCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SGComponents/CombatComponent.h"
#include "SGComponents/LockonComponent.h"

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
	if(SGCharacter->GetLockonComponent())
	{
		bIsLockedOnTarget = SGCharacter->GetLockonComponent()->bIsLockedOnTarget;
	}
	if(SGCharacter->GetCombatComponent())
	{
		bWeaponEquipped = SGCharacter->GetCombatComponent()->EquippedWeapon ? true : false;
		bIsBlocking = SGCharacter->GetCombatComponent()->bIsBlocking;
	}
	LockedonDirection = UKismetAnimationLibrary::CalculateDirection(SGCharacter->GetVelocity(), SGCharacter->GetActorRotation());
}
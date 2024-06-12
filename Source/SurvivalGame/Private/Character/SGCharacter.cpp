// Copyright Marco Freemantle

#include "Character/SGCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Interfaces/LockonInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

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

void ASGCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASGCharacter, LockonTarget);
	DOREPLIFETIME(ASGCharacter, bIsLockedOnTarget);
}

void ASGCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ASGCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if(LockonTarget && IsLocallyControlled())
	{
		const FRotator PlayerRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(),LockonTarget->GetActorLocation());
		GetController()->SetControlRotation(PlayerRot);
	}
}

void ASGCharacter::FindLockonTargets()
{
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(750.0f);
	TArray<FHitResult> OutResults;
	GetWorld()->SweepMultiByChannel(OutResults, GetActorLocation(), GetActorLocation(), FQuat::Identity, ECC_Pawn, SphereShape);

	LockonTargets.Empty();
	for (const auto& Hit : OutResults)
	{
		if (Hit.GetActor()->Implements<ULockonInterface>())
		{
			LockonTargets.AddUnique(Hit.GetActor());
		}
	}
}

void ASGCharacter::EngageLockon()
{
	FindLockonTargets();
	
	float MinDistance = FLT_MAX;

	for (const auto& Target : LockonTargets)
	{
		float CurrentDistance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
		if (CurrentDistance < MinDistance)
		{
			MinDistance = CurrentDistance;
			
			LockonTarget = Target;
			ServerSetLockonTarget(LockonTarget);
		}
	}

	if (LockonTarget)
	{
		bIsLockedOnTarget = true;
		if (AController* SGController = GetController())
		{
			SGController->SetIgnoreLookInput(true);
		}
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = true;

		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(BreakLockonTimer, this, &ASGCharacter::CheckLockonDistance, 0.15f, true);
		}
	}
}


void ASGCharacter::DisengageLockon()
{
	LockonTargets.Empty();
	LockonTarget = nullptr;
	ServerSetLockonTarget(nullptr);

	bIsLockedOnTarget = false;

	if (AController* SGController = GetController())
	{
		SGController->ResetIgnoreLookInput();
	}
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
}

void ASGCharacter::SwitchLockonTarget(bool bSwitchLeft)
{
	if (!LockonTarget) return;

	FindLockonTargets();
	AActor* CurrentLockonTarget = LockonTarget;
	LockonTargets.Remove(CurrentLockonTarget);
	float SwitchDistance = FLT_MAX;
	bool bSwitchedSuccessfully = false;

	for (const auto PotentialTarget : LockonTargets)
	{
		float DotProd = FVector::DotProduct(GetActorRightVector(), (GetActorLocation() - PotentialTarget->GetActorLocation()).GetSafeNormal());
		if ((bSwitchLeft && DotProd > 0) || (!bSwitchLeft && DotProd < 0))
		{
			float Distance = FVector::Distance(GetActorLocation(), PotentialTarget->GetActorLocation());
			if (Distance < SwitchDistance)
			{
				SwitchDistance = Distance;
				LockonTarget = PotentialTarget;
				ServerSetLockonTarget(LockonTarget);
				bSwitchedSuccessfully = true;
			}
		}
	}

	if (bSwitchedSuccessfully) LockonTargets.Add(CurrentLockonTarget);
}


void ASGCharacter::SwitchLockonTargetLeft()
{
	SwitchLockonTarget(true);
}

void ASGCharacter::SwitchLockonTargetRight()
{
	SwitchLockonTarget(false);
}

void ASGCharacter::ServerSetLockonTarget_Implementation(AActor* NewLockonTarget)
{
	LockonTarget = NewLockonTarget;
	bIsLockedOnTarget = LockonTarget ? true : false;
	if(bIsLockedOnTarget)
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
		bIsLockedOnTarget = true;
	}
	else
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		bIsLockedOnTarget = false;
	}
}

void ASGCharacter::CheckLockonDistance()
{
	if(LockonTarget)
	{
		if(FVector::Dist(GetActorLocation(), LockonTarget->GetActorLocation()) > 1000)
		{
			DisengageLockon();
			BreakLockonTimer.Invalidate();
		}
	}
}


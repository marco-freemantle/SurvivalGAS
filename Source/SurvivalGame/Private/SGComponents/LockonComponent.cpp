// Copyright Marco Freemantle


#include "SGComponents/LockonComponent.h"
#include "Character/SGCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/LockonInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

ULockonComponent::ULockonComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void ULockonComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void ULockonComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(LockonTarget && Character && Character->IsLocallyControlled())
	{
		const FRotator PlayerRot = UKismetMathLibrary::FindLookAtRotation(Character->GetActorLocation(),LockonTarget->GetActorLocation());
		Character->GetController()->SetControlRotation(PlayerRot);
	}
}

void ULockonComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULockonComponent, LockonTarget);
	DOREPLIFETIME(ULockonComponent, bIsLockedOnTarget);
}

void ULockonComponent::FindLockonTargets()
{
	if(!Character) return;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(750.0f);
	TArray<FHitResult> OutResults;
	GetWorld()->SweepMultiByChannel(OutResults, Character->GetActorLocation(), Character->GetActorLocation(), FQuat::Identity, ECC_Pawn, SphereShape);

	LockonTargets.Empty();
	for (const auto& Hit : OutResults)
	{
		if (Hit.GetActor()->Implements<ULockonInterface>())
		{
			LockonTargets.AddUnique(Hit.GetActor());
		}
	}
}

void ULockonComponent::EngageLockon()
{
	if(!Character) return;
	FindLockonTargets();
	
	float MinDistance = FLT_MAX;

	for (const auto& Target : LockonTargets)
	{
		float CurrentDistance = FVector::Dist(Character->GetActorLocation(), Target->GetActorLocation());
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
		if (AController* SGController = Character->GetController())
		{
			SGController->SetIgnoreLookInput(true);
		}
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->GetCharacterMovement()->bUseControllerDesiredRotation = true;

		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(BreakLockonTimer, this, &ULockonComponent::CheckLockonDistance, 0.15f, true);
		}
	}
}


void ULockonComponent::DisengageLockon()
{
	if(!Character) return;
	LockonTargets.Empty();
	LockonTarget = nullptr;
	ServerSetLockonTarget(nullptr);

	bIsLockedOnTarget = false;

	if (AController* SGController = Character->GetController())
	{
		SGController->ResetIgnoreLookInput();
	}
	
	Character->GetCharacterMovement()->bOrientRotationToMovement = true;
	Character->GetCharacterMovement()->bUseControllerDesiredRotation = false;
}

void ULockonComponent::SwitchLockonTarget(bool bSwitchLeft)
{
	if (!LockonTarget || !Character) return;

	FindLockonTargets();
	AActor* CurrentLockonTarget = LockonTarget;
	LockonTargets.Remove(CurrentLockonTarget);
	float SwitchDistance = FLT_MAX;
	bool bSwitchedSuccessfully = false;

	for (const auto PotentialTarget : LockonTargets)
	{
		float DotProd = FVector::DotProduct(Character->GetActorRightVector(), (Character->GetActorLocation() - PotentialTarget->GetActorLocation()).GetSafeNormal());
		if ((bSwitchLeft && DotProd > 0) || (!bSwitchLeft && DotProd < 0))
		{
			float Distance = FVector::Distance(Character->GetActorLocation(), PotentialTarget->GetActorLocation());
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


void ULockonComponent::SwitchLockonTargetLeft()
{
	SwitchLockonTarget(true);
}

void ULockonComponent::SwitchLockonTargetRight()
{
	SwitchLockonTarget(false);
}

void ULockonComponent::ServerSetLockonTarget_Implementation(AActor* NewLockonTarget)
{
	if(!Character) return;
	LockonTarget = NewLockonTarget;
	bIsLockedOnTarget = LockonTarget ? true : false;
	if(bIsLockedOnTarget)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->GetCharacterMovement()->bUseControllerDesiredRotation = true;
		bIsLockedOnTarget = true;
	}
	else
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = true;
		Character->GetCharacterMovement()->bUseControllerDesiredRotation = false;
		bIsLockedOnTarget = false;
	}
}

void ULockonComponent::CheckLockonDistance()
{
	if(LockonTarget && Character)
	{
		if(FVector::Dist(Character->GetActorLocation(), LockonTarget->GetActorLocation()) > 1000)
		{
			DisengageLockon();
			BreakLockonTimer.Invalidate();
		}
	}
}




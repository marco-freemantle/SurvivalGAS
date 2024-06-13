// Copyright Marco Freemantle

#include "SGComponents/CombatComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/SGCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/SGPlayerController.h"
#include "SGTypes/CombatState.h"
#include "Weapon/Weapon.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, CombatState);
}

// Called on the Server from ASGCharacter
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if(Character == nullptr || WeaponToEquip == nullptr) return;

	if(EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
	{
		EquipSecondaryWeapon(WeaponToEquip);
	}
	else
	{
		EquipPrimaryWeapon(WeaponToEquip);
	}
}

void UCombatComponent::DropWeapon()
{
	if(Character == nullptr) return;
	if(EquippedWeapon)
	{
		EquippedWeapon->Dropped();
		EquippedWeapon->GetWeaponMesh()->AddImpulse(Character->GetFollowCamera()->GetForwardVector() * 600.f, FName(), true);
		CombatState = ECombatState::ECS_Unoccupied;
		PlayDropWeaponSound(EquippedWeapon);
	}
	Controller = Controller == nullptr ? Cast<ASGPlayerController>(Character->Controller) : Controller;
	EquippedWeapon = nullptr;
	if(SecondaryWeapon)
	{
		EquipWeapon(SecondaryWeapon);
		SecondaryWeapon = nullptr;
	}
}

void UCombatComponent::SwapWeapons()
{
	CombatState = ECombatState::ECS_SwappingWeapons;
	if(Character)
	{
		Character->PlaySwapWeaponsMontage();
	}
	FTimerHandle SwapWeaponDelayTimer;
	Character->GetWorldTimerManager().SetTimer(SwapWeaponDelayTimer, [this]()
	{
		AWeapon* TempWeapon = EquippedWeapon;
		EquippedWeapon = SecondaryWeapon;
		SecondaryWeapon = TempWeapon;
	}, 0.5f, false);
}

void UCombatComponent::PlayEquipWeaponSound(const AWeapon* WeaponToEquip) const
{
	if(WeaponToEquip->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponToEquip->EquipSound, WeaponToEquip->GetActorLocation());
	}
}

void UCombatComponent::PlayDropWeaponSound(const AWeapon* WeaponToDrop) const
{
	if(WeaponToDrop->DropSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponToDrop->DropSound, WeaponToDrop->GetActorLocation());
	}
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if(EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
	CombatState = ECombatState::ECS_Unoccupied;
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	PlayEquipWeaponSound(EquippedWeapon);
	EquippedWeapon->SetOwner(Character);
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	SecondaryWeapon->SetOwner(Character);
	AttachActorToBack(WeaponToEquip);
	PlayEquipWeaponSound(SecondaryWeapon);
}

void UCombatComponent::OnRep_EquippedWeapon(const AWeapon* OldWeapon)
{
	if(EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		if(!SecondaryWeapon)
		{
			PlayEquipWeaponSound(EquippedWeapon);
		}
	}
	if(!EquippedWeapon && Character)
	{
		Controller = Controller == nullptr ? Cast<ASGPlayerController>(Character->Controller) : Controller;
		if(OldWeapon)
		{
			PlayDropWeaponSound(OldWeapon);
		}
	}
	if(OldWeapon && Character && Character->IsLocallyControlled())
	{
		OldWeapon->GetWeaponMesh()->SetVisibility(true);
	}
}

void UCombatComponent::OnRep_SecondaryWeapon(const AWeapon* OldWeapon)
{
	if(!OldWeapon) PlayEquipWeaponSound(SecondaryWeapon);

	if(SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		AttachActorToBack(SecondaryWeapon);
	}
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach) const
{
	if(const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket")))
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach) const
{
	if(const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("LeftHandSocket")))
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToBack(AActor* ActorToAttach) const
{
	if(const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("SecondaryWeaponSocket")))
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::FinishSwapWeapons()
{
	if(Character && Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

void UCombatComponent::FinishSwapAttachWeapon()
{
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	PlayEquipWeaponSound(EquippedWeapon);

	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBack(SecondaryWeapon);
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Unoccupied:
		break;
	case ECombatState::ECS_Attacking:
		if(Character && !Character->IsLocallyControlled())
		{
			
		}
		break;
	case ECombatState::ECS_SwappingWeapons:
		if(Character && !Character->IsLocallyControlled())
		{
			Character->PlaySwapWeaponsMontage();
		}
		break;
	}
}

bool UCombatComponent::ShouldSwapWeapons() const
{
	return (EquippedWeapon != nullptr && SecondaryWeapon != nullptr && CombatState != ECombatState::ECS_Attacking);
}


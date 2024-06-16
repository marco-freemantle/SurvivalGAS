// Copyright Marco Freemantle

#include "SGComponents/CombatComponent.h"
#include "Character/SGCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "SGTypes/CombatState.h"
#include "Weapon/Weapon.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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
	DOREPLIFETIME(UCombatComponent, PrimaryWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, Shield);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, bIsBlocking);
	DOREPLIFETIME(UCombatComponent, bIsShieldDrawn);
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if(Character == nullptr || WeaponToEquip == nullptr) return;

	if(WeaponToEquip->GetWeaponType() == EWeaponType::EWT_Shield && !Shield)
	{
		EquipShield(WeaponToEquip);
	}
	else if(PrimaryWeapon != nullptr && SecondaryWeapon == nullptr)
	{
		EquipSecondaryWeapon(WeaponToEquip);
	}
	else if(PrimaryWeapon == nullptr && SecondaryWeapon == nullptr)
	{
		EquipPrimaryWeapon(WeaponToEquip);
	}
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if(EquippedWeapon) return;
	CombatState = ECombatState::ECS_Unoccupied;
	PrimaryWeapon = WeaponToEquip;
	PrimaryWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	PrimaryWeapon->SetOwner(Character);
	Attach1HSwordToSide(PrimaryWeapon);
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	SecondaryWeapon->SetOwner(Character);
	AttachActorToBack(WeaponToEquip);
}

void UCombatComponent::EquipShield(AWeapon* WeaponToEquip)
{
	CombatState = ECombatState::ECS_Unoccupied;
	Shield = WeaponToEquip;
	Shield->SetWeaponState(EWeaponState::EWS_Equipped);
	Shield->SetOwner(Character);

	// If 1H sword is already drawn then draw shield
	if(EquippedWeapon && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_1HSword)
	{
		AttachActorToLeftHand(Shield);
		bIsShieldDrawn = true;
	}
	else
	{
		AttachShieldToBack(Shield);
	}
}

void UCombatComponent::DrawPrimaryWeapon()
{
	// Weapon already drawn -> sheath it
	if(PrimaryWeapon && EquippedWeapon)
	{
		if(EquippedWeapon->GetWeaponType() == EWeaponType::EWT_1HSword && Shield)
		{
			Attach1HSwordToSide(EquippedWeapon);
			AttachShieldToBack(Shield);
			EquippedWeapon = nullptr;
			bIsShieldDrawn = false;
		}
	}
	// Primary not draw -> draw it
	else if(PrimaryWeapon && !EquippedWeapon)
	{
		EquippedWeapon = PrimaryWeapon;
		AttachActorToRightHand(EquippedWeapon);
		// Draw shield as well if 1H Sword
		if(EquippedWeapon->GetWeaponType() == EWeaponType::EWT_1HSword && Shield)
		{
			AttachActorToLeftHand(Shield);
			bIsShieldDrawn = true;
		}
	}
}

void UCombatComponent::DrawSecondaryWeapon()
{
}

void UCombatComponent::Attack()
{
	if(!EquippedWeapon) return;
	switch (AttackCombo)
	{
	case 0:
		if(CombatState == ECombatState::ECS_Unoccupied && Character)
		{
			AttackCombo = 1;
			CombatState = ECombatState::ECS_Attacking;
			Character->MulticastPlayAttackMontage(AttackAMontage);
		}
		break;
	case 1:
		if(CombatState == ECombatState::ECS_Unoccupied && Character)
		{
			AttackCombo = 2;
			CombatState = ECombatState::ECS_Attacking;

			Character->MulticastPlayAttackMontage(AttackBMontage);
		}
		break;
	case 2:
		if(CombatState == ECombatState::ECS_Unoccupied && Character)
		{
			AttackCombo = 0;
			CombatState = ECombatState::ECS_Attacking;

			Character->MulticastPlayAttackMontage(AttackCMontage);
		}
		break;
	}
}

void UCombatComponent::Block()
{
	if(!Shield || !bIsShieldDrawn) return;
	bIsBlocking = true;
	if(Character && Character->GetMovementComponent())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BlockWalkSpeed;
	}
}

void UCombatComponent::Unblock()
{
	bIsBlocking = false;
	if(Character && Character->GetMovementComponent())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
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

void UCombatComponent::ResetCombo()
{
	AttackCombo = 0;
}

void UCombatComponent::AttackFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
	Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void UCombatComponent::OnRep_EquippedWeapon(const AWeapon* OldWeapon)
{
	if(EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
	}
}

void UCombatComponent::OnRep_PrimaryWeapon(const AWeapon* OldWeapon)
{
}

void UCombatComponent::OnRep_SecondaryWeapon(const AWeapon* OldWeapon)
{
	if(SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		AttachActorToBack(SecondaryWeapon);
	}
}

void UCombatComponent::OnRep_Shield(const AWeapon* OldShield)
{
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
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToBack(AActor* ActorToAttach) const
{
	if(const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("SecondaryWeaponSocket")))
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachShieldToBack(AActor* ActorToAttach) const
{
	if(const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("ShieldSocket")))
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::Attach1HSwordToSide(AActor* ActorToAttach) const
{
	if(const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("1HSwordSocket")))
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


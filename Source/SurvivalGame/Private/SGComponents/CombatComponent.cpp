// Copyright Marco Freemantle

#include "SGComponents/CombatComponent.h"
#include "Character/SGCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
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
	if (WeaponToEquip == nullptr) return;
	CombatState = ECombatState::ECS_Unoccupied;
	PrimaryWeapon = WeaponToEquip;
	PrimaryWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	PrimaryWeapon->SetOwner(Character);
	if(PrimaryWeapon->GetWeaponType() == EWeaponType::EWT_1HSword)
	{
		Attach1HToSide(PrimaryWeapon);
	}
	else
	{
		Attach2HToBack(PrimaryWeapon);
	}
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	SecondaryWeapon->SetOwner(Character);
	if(SecondaryWeapon->GetWeaponType() == EWeaponType::EWT_1HSword)
	{
		Attach1HToBack(SecondaryWeapon);
	}
	else
	{
		Attach2HToBack(SecondaryWeapon);
	}
}

void UCombatComponent::EquipShield(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	CombatState = ECombatState::ECS_Unoccupied;
	Shield = WeaponToEquip;
	Shield->SetWeaponState(EWeaponState::EWS_Equipped);
	Shield->SetOwner(Character);

	// If 1H sword is already drawn then draw shield
	if(EquippedWeapon && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_1HSword)
	{
		AttachShieldToLeftHand(Shield);
		bIsShieldDrawn = true;
	}
	else
	{
		AttachShieldToBack(Shield);
	}
}

void UCombatComponent::DrawPrimaryWeapon()
{
	// Primary already drawn -> sheath it
	if(PrimaryWeapon && EquippedWeapon && PrimaryWeapon == EquippedWeapon)
	{
		// 1H Sword & Shield
		if(EquippedWeapon->GetWeaponType() == EWeaponType::EWT_1HSword && Shield)
		{
			Character->MulticastPlaySheath1HSwordAndShieldMontage();
		}
		
		return;
	}
	// Secondary already drawn -> sheath it
	if(SecondaryWeapon && EquippedWeapon && SecondaryWeapon == EquippedWeapon)
	{
		
	}
	// Draw primary weapon
	if(PrimaryWeapon && !EquippedWeapon)
	{
		// 1H Sword & Shield
		if(PrimaryWeapon->GetWeaponType() == EWeaponType::EWT_1HSword && Shield)
		{
			Character->MulticastPlayDraw1HSwordAndShieldMontage();
		}
	}
}

void UCombatComponent::DrawSecondaryWeapon()
{
	// Secondary already drawn -> sheath it
	if(SecondaryWeapon && EquippedWeapon && SecondaryWeapon == EquippedWeapon)
	{
		
		
		return;
	}
	// Primary already drawn -> sheath it
	if(PrimaryWeapon && EquippedWeapon && PrimaryWeapon == EquippedWeapon)
	{
		
	}
	// Draw Secondary weapon
	if(SecondaryWeapon && !EquippedWeapon)
	{
		if(SecondaryWeapon->GetWeaponType() == EWeaponType::EWT_1HSword && Shield)
		{
			Character->MulticastPlayDraw1HSwordAndShieldMontage();
		}
	}
}

void UCombatComponent::Attack()
{
	if(!EquippedWeapon || Character->GetCharacterMovement()->IsFalling()) return;
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
	}
}

void UCombatComponent::OnRep_PrimaryWeapon(const AWeapon* OldWeapon)
{
	if(PrimaryWeapon && Character)
	{
		PrimaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedPrimary);
	}
}

void UCombatComponent::OnRep_SecondaryWeapon(const AWeapon* OldWeapon)
{
	if(SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	}
}

void UCombatComponent::Attach1HSwordToRightHand(AActor* ActorToAttach) const
{
	if(const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("1HSwordRightHandSocket")))
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachShieldToLeftHand(AActor* ActorToAttach) const
{
	if(const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("LeftHandSocket")))
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

void UCombatComponent::Attach2HToBack(AActor* ActorToAttach) const
{
	if(const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("2HSocket")))
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::Attach1HToSide(AActor* ActorToAttach) const
{
	if(const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("1HSideSocket")))
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::Attach1HToBack(AActor* ActorToAttach) const
{
	if(const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("1HBackSocket")))
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::UnSheathAttach1HSwordAndShield()
{
	if(Character && Character->HasAuthority() && PrimaryWeapon && Shield)
	{
		EquippedWeapon = PrimaryWeapon;
		Attach1HSwordToRightHand(PrimaryWeapon);
		AttachShieldToLeftHand(Shield);
		bIsShieldDrawn = true;
	}
}

void UCombatComponent::SheathAttach1HSwordAndShield()
{
	if(Character && Character->HasAuthority() && EquippedWeapon && Shield)
	{
		Attach1HToSide(EquippedWeapon);
		FTimerHandle SheathTimer;
		GetWorld()->GetTimerManager().SetTimer(SheathTimer, [this]()
		{
			EquippedWeapon = nullptr;
			AttachShieldToBack(Shield);
			bIsShieldDrawn = false;
		}, 0.5f, false);
	}
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
			
		}
		break;
	}
}


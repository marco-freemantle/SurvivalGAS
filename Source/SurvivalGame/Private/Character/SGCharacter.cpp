// Copyright Marco Freemantle

#include "Character/SGCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "SGComponents/CombatComponent.h"
#include "SGComponents/LockonComponent.h"
#include "Weapon/Weapon.h"

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

	LockonComponent = CreateDefaultSubobject<ULockonComponent>(TEXT("LockonComponent"));
	LockonComponent->SetIsReplicated(true);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void ASGCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASGCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void ASGCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if(LockonComponent)
	{
		LockonComponent->Character = this;
	}
	if(CombatComponent)
	{
		CombatComponent->Character = this;
	}
}

void ASGCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ASGCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASGCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if(OverlappingWeapon && IsLocallyControlled())
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	
	OverlappingWeapon = Weapon;
	
	if(IsLocallyControlled() && OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
}

void ASGCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if(LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void ASGCharacter::InteractButtonPressed()
{
	ServerInteract();
}

void ASGCharacter::AttackButtonPressed()
{
	if(CombatComponent->CombatState != ECombatState::ECS_Attacking && !HasAuthority())
	{
		CombatComponent->Attack();
	}
	ServerAttack();
}

void ASGCharacter::BlockButtonPressed()
{
	if(!HasAuthority())
	{
		CombatComponent->Block();
	}
	ServerBlock();
}

void ASGCharacter::BlockButtonReleased()
{
	if(!HasAuthority())
	{
		CombatComponent->Unblock();
	}
	ServerUnblock();
}

void ASGCharacter::DrawPrimaryButtonPressed()
{
	if(!HasAuthority())
	{
		CombatComponent->DrawPrimaryWeapon();
	}
	ServerDrawPrimary();
}

void ASGCharacter::DrawSecondaryButtonPressed()
{
	if(!HasAuthority())
	{
		CombatComponent->DrawSecondaryWeapon();
	}
	ServerDrawSecondary();
}

void ASGCharacter::ServerInteract_Implementation()
{
	if(OverlappingWeapon && CombatComponent)
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
}

void ASGCharacter::ServerAttack_Implementation()
{
	if(CombatComponent)
	{
		CombatComponent->Attack();
	}
}

void ASGCharacter::ServerBlock_Implementation()
{
	if(CombatComponent)
	{
		CombatComponent->Block();
	}
}

void ASGCharacter::ServerUnblock_Implementation()
{
	if(CombatComponent)
	{
		CombatComponent->Unblock();
	}
}

void ASGCharacter::ServerDrawPrimary_Implementation()
{
	CombatComponent->DrawPrimaryWeapon();
}

void ASGCharacter::ServerDrawSecondary_Implementation()
{
	CombatComponent->DrawSecondaryWeapon();
}

void ASGCharacter::MulticastPlayAttackMontage_Implementation(UAnimMontage* Montage)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage);
	}
}

void ASGCharacter::MulticastPlayDraw1HSwordAndShieldMontage_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && Draw1HSwordAndShieldMontage)
	{
		AnimInstance->Montage_Play(Draw1HSwordAndShieldMontage);
	}
}

void ASGCharacter::MulticastPlaySheath1HSwordAndShieldMontage_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && Draw1HSwordAndShieldMontage)
	{
		AnimInstance->Montage_Play(Sheath1HSwordAndShieldMontage);
	}
}




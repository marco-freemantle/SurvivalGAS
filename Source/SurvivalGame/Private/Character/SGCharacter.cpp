// Copyright Marco Freemantle

#include "Character/SGCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "HUD/SGHUD.h"
#include "Interfaces/InteractInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "SGComponents/CombatComponent.h"
#include "SGComponents/InventoryComponent.h"
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
	GetCharacterMovement()->NavAgentProps.bCanCrouch = false;

	LockonComponent = CreateDefaultSubobject<ULockonComponent>(TEXT("LockonComponent"));
	LockonComponent->SetIsReplicated(true);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	InventoryComponent->SetIsReplicated(true);

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void ASGCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASGCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ASGCharacter, OverlappingInteractable, COND_OwnerOnly);
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
	if(InventoryComponent)
	{
		InventoryComponent->Character = this;
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

void ASGCharacter::SetOverlappingInteractable(AActor* Interactable)
{
	if(OverlappingInteractable && IsLocallyControlled())
	{
		if(IInteractInterface* InteractInterface = Cast<IInteractInterface>(OverlappingInteractable))
		{
			InteractInterface->ShowPickupWidget(false);
		}
	}
	
	OverlappingInteractable = Interactable;
	
	if(IsLocallyControlled() && OverlappingInteractable)
	{
		if(IInteractInterface* InteractInterface = Cast<IInteractInterface>(OverlappingInteractable))
		{
			InteractInterface->ShowPickupWidget(true);
		}
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

void ASGCharacter::OnRep_OverlappingInteractable(AActor* LastInteractable)
{
	if(OverlappingInteractable)
	{
		if(IInteractInterface* InteractInterface = Cast<IInteractInterface>(OverlappingInteractable))
		{
			InteractInterface->ShowPickupWidget(true);
		}
	}
	if(LastInteractable)
	{
		if(IInteractInterface* InteractInterface = Cast<IInteractInterface>(LastInteractable))
		{
			InteractInterface->ShowPickupWidget(false);
		}
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

void ASGCharacter::RollButtonPressed(const FName& Direction)
{
	if(CombatComponent->CombatState != ECombatState::ECS_Unoccupied && !HasAuthority())
	{
		CombatComponent->Roll(Direction);
	}
	ServerRoll(Direction);
}

void ASGCharacter::DodgeButtonPressed(const FName& Direction)
{
	ServerDodge(Direction);
}

void ASGCharacter::ServerInteract_Implementation()
{
	if(InventoryComponent && OverlappingWeapon)
	{
		InventoryComponent->ServerInteract(OverlappingWeapon);
	}
	if(InventoryComponent && OverlappingInteractable)
	{
		InventoryComponent->ServerInteract(OverlappingInteractable);
	}
}

void ASGCharacter::TryEquipWeapons(FSlotStruct PrimaryWeaponSlot, FSlotStruct ShieldSlot) const
{
    if(!InventoryComponent || !InventoryComponent->DataTable || !CombatComponent) return;
    
    static const FString ContextString(TEXT("Item Data Context"));
    
    auto EquipWeapon = [&](const FSlotStruct Slot, AWeapon*& WeaponSlot, const bool isPrimary = false, const bool isShield = false)
    {
        if(const FItemStruct* ItemData = InventoryComponent->DataTable->FindRow<FItemStruct>(Slot.ItemID, ContextString, true))
        {
            if((isShield && ItemData->ItemType == EItemType::EIT_Shield) || (!isShield && ItemData->ItemType == EItemType::EIT_Weapon))
            {
                if(WeaponSlot) WeaponSlot->Destroy();
                const FActorSpawnParameters SpawnParameters;
                if(AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ItemData->ItemClass, FTransform(), SpawnParameters))
                {
                    if(AWeapon* WeaponCast = Cast<AWeapon>(SpawnedActor))
                    {
                        WeaponSlot = WeaponCast;
                        if(isPrimary)
                        {
                            CombatComponent->EquipPrimaryWeapon(WeaponCast);
                        }
                        else if(isShield)
                        {
                            CombatComponent->EquipShield(WeaponCast);
                        }
                        return;
                    }
                    SpawnedActor->Destroy();
                }
            }
        }
        if(WeaponSlot)
        {
            if(isPrimary && CombatComponent->EquippedWeapon == WeaponSlot)
            {
                CombatComponent->EquippedWeapon = nullptr;
                CombatComponent->bIsShieldDrawn = false;
            }
            WeaponSlot->Destroy();
            WeaponSlot = nullptr;
        }
        if(isShield)
        {
            CombatComponent->bIsShieldDrawn = false;
        }
    };
    
    EquipWeapon(PrimaryWeaponSlot, CombatComponent->PrimaryWeapon, true);
    EquipWeapon(ShieldSlot, CombatComponent->Shield, false, true);
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
	if(CombatComponent)
	{
		CombatComponent->DrawPrimaryWeapon();
	}
}

void ASGCharacter::ServerRoll_Implementation(const FName& Direction)
{
	if(CombatComponent)
	{
		CombatComponent->Roll(Direction);
	}}

void ASGCharacter::ServerDodge_Implementation(const FName& Direction)
{
	if(CombatComponent)
	{
		CombatComponent->Dodge(Direction);
	}}

void ASGCharacter::MulticastPlayAttackMontage_Implementation(UAnimMontage* Montage)
{
	if(UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage);
	}
}

void ASGCharacter::MulticastPlayDraw1HSwordAndShieldMontage_Implementation()
{
	if(UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance && Draw1HSwordAndShieldMontage)
	{
		AnimInstance->Montage_Play(Draw1HSwordAndShieldMontage);
	}
}

void ASGCharacter::MulticastPlaySheath1HSwordAndShieldMontage_Implementation()
{
	if(UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance && Draw1HSwordAndShieldMontage)
	{
		AnimInstance->Montage_Play(Sheath1HSwordAndShieldMontage);
	}
}

void ASGCharacter::MulticastPlayRollMontage_Implementation(const FName& Direction)
{
	if(UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance && RollMontage)
	{
		AnimInstance->Montage_Play(RollMontage);
		AnimInstance->Montage_JumpToSection(Direction);
	}
}

void ASGCharacter::MulticastPlayDodgeMontage_Implementation(const FName& Direction)
{
	if(UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance && DodgeMontage)
	{
		AnimInstance->Montage_Play(DodgeMontage);
		AnimInstance->Montage_JumpToSection(Direction);
	}
}

void ASGCharacter::ClientHideContainerWidget_Implementation(UInventoryComponent* ContainerInventoryComponent) const
{
	if(GetWorld() && GetWorld()->GetFirstPlayerController() && GetWorld()->GetFirstPlayerController()->GetHUD())
	{
		if(ASGHUD* SGHUD = Cast<ASGHUD>(GetWorld()->GetFirstPlayerController()->GetHUD()); SGHUD && SGHUD->bIsContainerOpen)
		{
			SGHUD->ToggleShowContainer(InventoryComponent);
		}
	}
}

void ASGCharacter::ClientPlayPickupSound_Implementation()
{
	if(PickupSound)
	{
		UGameplayStatics::PlaySound2D(this, PickupSound);
	}
}




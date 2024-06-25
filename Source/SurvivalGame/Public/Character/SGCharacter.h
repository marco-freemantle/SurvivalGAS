// Copyright Marco Freemantle

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/LockonInterface.h"
#include "SGCharacter.generated.h"

class UInventoryComponent;
class ULockonComponent;
class UCombatComponent;
class ASGPlayerController;
class USpringArmComponent;
class UCameraComponent;
class AWeapon;

UCLASS()
class SURVIVALGAME_API ASGCharacter : public ACharacter, public ILockonInterface
{
	GENERATED_BODY()

public:
	ASGCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

	void SetOverlappingWeapon(AWeapon* Weapon);
	void SetOverlappingInteractable(AActor* Interactable);

	void InteractButtonPressed();
	void AttackButtonPressed();
	void BlockButtonPressed();
	void BlockButtonReleased();
	void DrawPrimaryButtonPressed();
	void DrawSecondaryButtonPressed();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayAttackMontage(UAnimMontage* Montage);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayDraw1HSwordAndShieldMontage();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlaySheath1HSwordAndShieldMontage();

	UFUNCTION(Client, Reliable)
	void ClientHideContainerWidget(UInventoryComponent* ContainerInventoryComponent) const;

	UFUNCTION(Client, Unreliable)
	void ClientPlayPickupSound();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingInteractable)
	AActor* OverlappingInteractable;

	UPROPERTY()
	ASGPlayerController* VBPlayerController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	ULockonComponent* LockonComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCombatComponent* CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UInventoryComponent* InventoryComponent;
	
	UPROPERTY(EditAnywhere, Category=Combat)
	UAnimMontage* Draw1HSwordAndShieldMontage;
	
	UPROPERTY(EditAnywhere, Category=Combat)
	UAnimMontage* Sheath1HSwordAndShieldMontage;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION()
	void OnRep_OverlappingInteractable(AActor* LastInteractable);

	UFUNCTION(Server, Reliable)
	void ServerInteract();

	UFUNCTION(Server, Reliable)
	void ServerAttack();

	UFUNCTION(Server, Reliable)
	void ServerBlock();

	UFUNCTION(Server, Reliable)
	void ServerUnblock();

	UFUNCTION(Server, Reliable)
	void ServerDrawPrimary();

	UFUNCTION(Server, Reliable)
	void ServerDrawSecondary();

	UPROPERTY(EditAnywhere, Category=Audio)
	USoundBase* PickupSound;

public:
	FORCEINLINE ULockonComponent* GetLockonComponent() const { return LockonComponent; }
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return CombatComponent; }
	FORCEINLINE UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

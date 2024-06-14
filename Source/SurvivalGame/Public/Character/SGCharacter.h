// Copyright Marco Freemantle

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/LockonInterface.h"
#include "SGCharacter.generated.h"

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

	void PlaySwapWeaponsMontage() const;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayAttackMontage(UAnimMontage* Montage);

	void InteractButtonPressed();
	void DropEquippedWeaponButtonPressed();
	void SwapWeaponsButtonPressed();
	void AttackButtonPressed();
	void BlockButtonPressed();
	void BlockButtonReleased();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	UPROPERTY()
	ASGPlayerController* VBPlayerController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	ULockonComponent* LockonComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCombatComponent* CombatComponent;

	UPROPERTY(EditAnywhere, Category=Combat)
	UAnimMontage* SwapWeaponsMontage;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION(Server, Reliable)
	void ServerInteract();

	UFUNCTION(Server, Reliable)
	void ServerDropEquippedWeapon();

	UFUNCTION(Server, Reliable)
	void ServerSwapWeapons();

	UFUNCTION(Server, Reliable)
	void ServerAttack();

	UFUNCTION(Server, Reliable)
	void ServerBlock();

	UFUNCTION(Server, Reliable)
	void ServerUnblock();

public:
	FORCEINLINE ULockonComponent* GetLockonComponent() const { return LockonComponent; }
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return CombatComponent; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

// Copyright Marco Freemantle

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SGTypes/CombatState.h"
#include "CombatComponent.generated.h"

class AWeapon;
class ASGPlayerController;
class ASGCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SURVIVALGAME_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();

	friend class ASGCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(AWeapon* WeaponToEquip);
	void SwapWeapons();
	void PlayEquipWeaponSound(const AWeapon* WeaponToEquip) const;
	void Attack();
	void Block();
	void Unblock();

	void AttachActorToRightHand(AActor* ActorToAttach) const;
	void AttachActorToLeftHand(AActor* ActorToAttach) const;
	void AttachActorToBack(AActor* ActorToAttach) const;

	UFUNCTION(BlueprintCallable)
	void FinishSwapWeapons();

	UFUNCTION(BlueprintCallable)
	void FinishSwapAttachWeapon();

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon, BlueprintReadOnly)
	AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon, BlueprintReadOnly)
	AWeapon* SecondaryWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, BlueprintReadOnly)
	AWeapon* Shield;

	UPROPERTY(Replicated)
	bool bIsBlocking = false;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_EquippedWeapon(const AWeapon* OldWeapon);

	UFUNCTION()
	void OnRep_SecondaryWeapon(const AWeapon* OldWeapon);

	UFUNCTION()
	void OnRep_Shield(const AWeapon* OldShield);

	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);
	void EquipShield(AWeapon* WeaponToEquip);

private:
	UPROPERTY()
	ASGCharacter* Character;
	
	UPROPERTY()
	ASGPlayerController* Controller;	

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();
	
	int32 AttackCombo = 0;

	UFUNCTION(BlueprintCallable)
	void ResetCombo();

	UFUNCTION(BlueprintCallable)
	void AttackFinished();

	UPROPERTY(EditAnywhere, Category=Combat)
	UAnimMontage* AttackAMontage;

	UPROPERTY(EditAnywhere, Category=Combat)
	UAnimMontage* AttackBMontage;

	UPROPERTY(EditAnywhere, Category=Combat)
	UAnimMontage* AttackCMontage;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float BlockWalkSpeed;
	
public:
	bool ShouldSwapWeapons() const;
};

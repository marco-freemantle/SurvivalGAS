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
	
	void Attack();
	void Block();
	void Unblock();

	void DrawPrimaryWeapon();

	UPROPERTY(Replicated)
	bool bIsShieldDrawn = false;

	void Attach1HSwordToRightHand(AActor* ActorToAttach) const;
	void AttachShieldToLeftHand(AActor* ActorToAttach) const;
	void AttachShieldToBack(AActor* ActorToAttach) const;
	void Attach2HToBack(AActor* ActorToAttach) const;
	void Attach1HToSide(AActor* ActorToAttach) const;
	void Attach1HToBack(AActor* ActorToAttach) const;

	// Called from anim notify
	UFUNCTION(BlueprintCallable)
	void UnSheathAttach1HSwordAndShield();

	UFUNCTION(BlueprintCallable)
	void SheathAttach1HSwordAndShield();

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon, BlueprintReadOnly)
	AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_PrimaryWeapon, BlueprintReadOnly)
	AWeapon* PrimaryWeapon;

	UPROPERTY(Replicated, BlueprintReadOnly)
	AWeapon* Shield;

	UPROPERTY(Replicated)
	bool bIsBlocking = false;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_EquippedWeapon(const AWeapon* OldWeapon);

	UFUNCTION()
	void OnRep_PrimaryWeapon(const AWeapon* OldWeapon);

	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
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
	float BaseWalkSpeed = 450.f;

	UPROPERTY(EditAnywhere)
	float BlockWalkSpeed = 175.f;
	
public:
	FORCEINLINE ECombatState GetCombatState() const { return CombatState; }
};

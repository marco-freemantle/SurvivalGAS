// Copyright Marco Freemantle

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SGPlayerController.generated.h"

class ASGCharacter;
class ASGGameMode;
class ASGCharacter;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
/**
 * 
 */
UCLASS()
class SURVIVALGAME_API ASGPlayerController : public APlayerController
{
GENERATED_BODY()
	
public:
	ASGPlayerController();
	virtual void PlayerTick(float DeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	void PauseGame(const FInputActionValue& InputActionValue);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> SGContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> LookUpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> TurnAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> SwapWeaponsAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> PauseGameAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> LockonAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> SwitchLockonTargetLeftAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> SwitchLockonTargetRightAction;

	void Move(const FInputActionValue& InputActionValue);
	void LookUp(const FInputActionValue& InputActionValue);
	void Turn(const FInputActionValue& InputActionValue);
	void Jump(const FInputActionValue& InputActionValue);
	void Interact(const FInputActionValue& InputActionValue);
	void Crouch(const FInputActionValue& InputActionValue);
	void UnCrouch(const FInputActionValue& InputActionValue);
	void SwapWeapons(const FInputActionValue& InputActionValue);
	void Lockon(const FInputActionValue& InputActionValue);
	void SwitchLockonTargetLeft(const FInputActionValue& InputActionValue);
	void SwitchLockonTargetRight(const FInputActionValue& InputActionValue);
	void SetbCanEquipTrue();

	bool bCanEquip = true;

	UPROPERTY()
	ASGGameMode* SGGameMode;

	UPROPERTY()
	ASGCharacter* SGOwnerCharacter;
};

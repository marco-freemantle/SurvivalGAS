// Copyright Marco Freemantle

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SGTypes/TurningInPlace.h"
#include "SGCharacter.generated.h"

class ASGPlayerController;
class USpringArmComponent;
class UCameraComponent;
UCLASS()
class SURVIVALGAME_API ASGCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASGCharacter();
	virtual void Tick(float DeltaTime) override;

	void EngageLockon();
	void DisenganeLockon();
	bool bIsLockedOnTarget = false;

protected:
	virtual void BeginPlay() override;
	void AimOffset(float DeltaTime);

private:
	UPROPERTY()
	TArray<AActor*> LockonTargets;
	
	UPROPERTY()
	AActor* LockonTarget;

	bool bShouldRotate = false;
	
	FTimerHandle BreakLockonTimer;

	void CheckLockonDistance();
	
	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera;

	UPROPERTY()
	ASGPlayerController* VBPlayerController;

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;
	
	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

public:
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
};

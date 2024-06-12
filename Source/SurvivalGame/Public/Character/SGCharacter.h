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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void FindLockonTargets();
	void EngageLockon();
	void DisengageLockon();
	void SwitchLockonTarget(bool bSwitchLeft);
	void SwitchLockonTargetLeft();
	void SwitchLockonTargetRight();

	UPROPERTY(Replicated)
	bool bIsLockedOnTarget = false;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TArray<AActor*> LockonTargets;
	
	UPROPERTY(Replicated)
	AActor* LockonTarget;

	UFUNCTION(Server, Reliable)
	void ServerSetLockonTarget(AActor* NewLockonTarget);
	
	FTimerHandle BreakLockonTimer;

	void CheckLockonDistance();
	
	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera;

	UPROPERTY()
	ASGPlayerController* VBPlayerController;
};

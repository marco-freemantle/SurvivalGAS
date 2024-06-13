// Copyright Marco Freemantle

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LockonComponent.generated.h"

class ASGPlayerController;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SURVIVALGAME_API ULockonComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULockonComponent();

	friend class ASGCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
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
	ASGCharacter* Character;
	
	UPROPERTY()
	ASGPlayerController* Controller;

	UPROPERTY()
	TArray<AActor*> LockonTargets;
	
	UPROPERTY(Replicated)
	AActor* LockonTarget;

	UFUNCTION(Server, Reliable)
	void ServerSetLockonTarget(AActor* NewLockonTarget);
	
	FTimerHandle BreakLockonTimer;

	void CheckLockonDistance();
		
};

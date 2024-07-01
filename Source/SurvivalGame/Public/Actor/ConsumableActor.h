// Copyright Marco Freemantle

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/InteractInterface.h"
#include "ConsumableActor.generated.h"

class UGameplayEffect;
class UWidgetComponent;
class UItemDataComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class SURVIVALGAME_API AConsumableActor : public AActor, public IInteractInterface
{
	GENERATED_BODY()
	
public:	
	AConsumableActor();

	virtual void ShowPickupWidget(bool bShowWidget) override;
	virtual void InteractWith(ASGCharacter* SGCharacter) override;
	
	void ConsumeItem(AActor* TargetActor) const;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere, Category="Applied Effects")
	TSubclassOf<UGameplayEffect> GameplayEffectClass;

private:
	UPROPERTY(VisibleAnywhere, Category = "Pickup Properties")
	UStaticMeshComponent* Mesh;
	
	UPROPERTY(VisibleAnywhere, Category = "Pickup Properties")
	USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere, Category = "Pickup Properties")
	UWidgetComponent* InteractWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UItemDataComponent* ItemDataComponent;
};

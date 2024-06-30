// Copyright Marco Freemantle

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/InteractInterface.h"
#include "PickupActor.generated.h"

class UInventoryComponent;
class UWidgetComponent;
class UItemDataComponent;
class USphereComponent;
class USkeletalMeshComponent;

UCLASS()
class SURVIVALGAME_API APickupActor : public AActor, public IInteractInterface
{
	GENERATED_BODY()
	
public:	
	APickupActor();

	virtual void ShowPickupWidget(bool bShowWidget) override;
	virtual void InteractWith(ASGCharacter* SGCharacter) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	UPROPERTY(VisibleAnywhere, Category = "Pickup Properties")
	USkeletalMeshComponent* Mesh;
	
	UPROPERTY(VisibleAnywhere, Category = "Pickup Properties")
	USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UInventoryComponent* InventoryComponent;

	UPROPERTY(VisibleAnywhere, Category = "Pickup Properties")
	UWidgetComponent* InteractWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UItemDataComponent* ItemDataComponent;
};

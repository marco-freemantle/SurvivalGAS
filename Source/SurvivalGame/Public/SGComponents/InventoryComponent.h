// Copyright Marco Freemantle

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class ASGCharacter;

USTRUCT(BlueprintType)
struct FSlotStruct
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	FName ItemID;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	int32 Quantity = 0;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SURVIVALGAME_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventoryComponent();
	friend class ASGCharacter;

	void AddToInventory(FName ItemID, int32 Quantity);
	int32 FindSlot(FName ItemID);
	int32 GetMaxStackSize(FName ItemID) const;
	void AddToStack(int32 Index);
	int32 AnyEmptySlotsAvailable();
	bool CreateNewStack(FName ItemID);

	UPROPERTY(Replicated, BlueprintReadOnly)
	TArray<FSlotStruct> Content;
	
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void ServerInteract(AActor* Target);

private:
	UPROPERTY()
	ASGCharacter* Character;
	
	int32 InventorySize = 25;

	UPROPERTY(EditAnywhere)
	UDataTable* DataTable;
};

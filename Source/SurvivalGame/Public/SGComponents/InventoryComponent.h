// Copyright Marco Freemantle

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SGTypes/ItemStruct.h"
#include "InventoryComponent.generated.h"

class ASGCharacter;

USTRUCT(BlueprintType)
struct FSlotStruct
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Quantity = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInventoryUpdated);

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
	void TransferSlots(int32 SourceIndex, UInventoryComponent* SourceInventory, int32 DestinationIndex);
	void TransferEquippableSlots(int32 SourceIndex, UInventoryComponent* SourceInventory, EItemType ItemType);
	FItemStruct GetItemData(FName ItemID) const;

	void RemoveFromInventory(int32 Index, bool bRemoveWholeStack, bool bIsConsumed);
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerRemove(int32 Index, bool bRemoveWholeStack, bool bIsConsumed);

	UFUNCTION(Server, Reliable)
	void ServerDropItem(FName ItemID, int32 Quantity);

	UFUNCTION(Client, Reliable)
	void ClientOnLocalInteract(AActor* TargetActor, AActor* Interactor);

	UPROPERTY(Replicated, BlueprintReadOnly, EditAnywhere)
	TArray<FSlotStruct> Content;

	UPROPERTY(Replicated, BlueprintReadOnly, EditAnywhere)
	FSlotStruct MainWeaponSlot;

	UPROPERTY(Replicated, BlueprintReadOnly, EditAnywhere)
	FSlotStruct SecondaryWeaponSlot;

	UPROPERTY(Replicated, BlueprintReadOnly, EditAnywhere)
	FSlotStruct ShieldSlot;

	UPROPERTY(BlueprintAssignable)
	FInventoryUpdated OnInventoryUpdated;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastUpdateInventory();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerTransferSlots(int32 SourceIndex, UInventoryComponent* SourceInventory, int32 DestinationIndex);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerTransferEquippableSlots(int32 SourceIndex, UInventoryComponent* SourceInventory, EItemType ItemType);
	
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void ServerInteract(AActor* Target);

private:
	UPROPERTY()
	ASGCharacter* Character;
	
	int32 InventorySize = 30;

	UPROPERTY(EditAnywhere)
	UDataTable* DataTable;

	FVector GetDropLocation() const;
};

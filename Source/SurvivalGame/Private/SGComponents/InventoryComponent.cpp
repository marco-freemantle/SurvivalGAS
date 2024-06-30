// Copyright Marco Freemantle


#include "SGComponents/InventoryComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/SGAttributeSet.h"
#include "Character/SGCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "SGTypes/ItemStruct.h"
#include "SGComponents/ItemDataComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, Content);
	DOREPLIFETIME(UInventoryComponent, PrimaryWeaponSlot);
	DOREPLIFETIME(UInventoryComponent, ShieldSlot);
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	Content.SetNum(InventorySize);
}

void UInventoryComponent::ServerInteract_Implementation(AActor* Target)
{
	if(Target)
	{
		// Interacting with a pickup item
		if (UItemDataComponent* ItemDataComponent = Cast<UItemDataComponent>(Target->GetComponentByClass(UItemDataComponent::StaticClass())))
		{
			if(ASGCharacter* SGCharacter = Cast<ASGCharacter>(GetOwner()))
			{
				ItemDataComponent->InteractWith(SGCharacter);
			}
		}
		// Interacting with a generic interactable item
		if (ASGCharacter* SGCharacter = Cast<ASGCharacter>(GetOwner()))
		{
			Target->SetOwner(SGCharacter->GetController());

			ClientOnLocalInteract(Target, SGCharacter);
		}
	}
}

void UInventoryComponent::AddToInventory(FName ItemID, int32 Quantity)
{
	int32 QuantityRemaining = Quantity;
	bool bHasFailed = false;
	while (QuantityRemaining > 0 && !bHasFailed)
	{
		int32 FoundIndex = FindSlot(ItemID);
		if(FoundIndex >= 0)
		{
			AddToStack(FoundIndex);
			QuantityRemaining--;
		}
		else
		{
			if(AnyEmptySlotsAvailable() >= 0)
			{
				if(CreateNewStack(ItemID))
				{
					QuantityRemaining--;
				}
				else
				{
					bHasFailed = true;
				}
			}
			else
			{
				bHasFailed = true;
			}
		}
	}
	MulticastUpdateInventory();
}

int32 UInventoryComponent::FindSlot(FName ItemID)
{
	for (int32 Index = 0; Index < Content.Num(); ++Index)
	{
		const FSlotStruct Item = Content[Index];
		if (Item.ItemID == ItemID)
		{
			if (Item.Quantity < GetMaxStackSize(ItemID))
			{
				return Index;
			}
		}
	}
	return -1;
}

int32 UInventoryComponent::GetMaxStackSize(FName ItemID) const
{
	if(DataTable)
	{
		static const FString ContextString(TEXT("Item Data Context"));
		if(const FItemStruct* ItemData = DataTable->FindRow<FItemStruct>(ItemID, ContextString, true))
		{
			return ItemData->StackSize;
		}
		return -1;
	}
	return -1;
}

void UInventoryComponent::AddToStack(int32 Index)
{
	FSlotStruct NewSlotStruct;
	NewSlotStruct.ItemID = Content[Index].ItemID;
	NewSlotStruct.Quantity = Content[Index].Quantity += 1;
	Content[Index] = NewSlotStruct;
}

int32 UInventoryComponent::AnyEmptySlotsAvailable()
{
	for (int32 Index = 0; Index < Content.Num(); ++Index)
	{
		const FSlotStruct& Item = Content[Index];
		if (Item.Quantity == 0)
		{
			return Index;
		}
	}
	return -1;
}

bool UInventoryComponent::CreateNewStack(FName ItemID)
{
	if(int32 AvailableIndex = AnyEmptySlotsAvailable(); AvailableIndex >= 0)
	{
		FSlotStruct NewSlotStruct;
		NewSlotStruct.ItemID = ItemID;
		NewSlotStruct.Quantity = 1;
		Content[AvailableIndex] = NewSlotStruct;
		return true;
	}
	return false;
}

// Only called if dropping items between different slots
void UInventoryComponent::TransferSlots(int32 SourceIndex, UInventoryComponent* SourceInventory, int32 DestinationIndex)
{
	if(SourceInventory)
	{
		const FSlotStruct SlotContent = SourceInventory->Content[SourceIndex];

		// Items are the same type -> try stack
		if(Content[DestinationIndex].ItemID == SlotContent.ItemID)
		{
			int32 RemainingItems = FMath::Clamp((Content[DestinationIndex].Quantity + SlotContent.Quantity) - GetMaxStackSize(SlotContent.ItemID), 0, GetMaxStackSize(SlotContent.ItemID));
			// Items remaining after stack
			if(RemainingItems > 0)
			{
				FSlotStruct NewSlotStruct;
				NewSlotStruct.ItemID = SlotContent.ItemID;
				NewSlotStruct.Quantity = RemainingItems;
				SourceInventory->Content[SourceIndex] = NewSlotStruct;

				NewSlotStruct.Quantity = FMath::Clamp(Content[DestinationIndex].Quantity + SlotContent.Quantity, 0, GetMaxStackSize(SlotContent.ItemID));
				Content[DestinationIndex] = NewSlotStruct;
				
				MulticastUpdateInventory();
				SourceInventory->MulticastUpdateInventory();
			}
			// No items remaining after stack
			else
			{
				const FSlotStruct NewSourceSlotStruct;
				SourceInventory->Content[SourceIndex] = NewSourceSlotStruct;

				FSlotStruct NewDestinationSlotStruct = Content[DestinationIndex];
				NewDestinationSlotStruct.Quantity = FMath::Clamp(Content[DestinationIndex].Quantity + SlotContent.Quantity, 0, GetMaxStackSize(SlotContent.ItemID));
				Content[DestinationIndex] = NewDestinationSlotStruct;
				
				MulticastUpdateInventory();
				SourceInventory->MulticastUpdateInventory();
			}
		}
		// Items are not the same type
		else
		{
			SourceInventory->Content[SourceIndex] = Content[DestinationIndex];
			Content[DestinationIndex] = SlotContent;

			MulticastUpdateInventory();
			SourceInventory->MulticastUpdateInventory();
		}
	}
	if(Character) Character->TryEquipWeapons(PrimaryWeaponSlot, ShieldSlot);
}

// Only called if dropping items between different slots
void UInventoryComponent::TransferEquippableSlots(int32 SourceIndex, int32 DestinationIndex, UInventoryComponent* SourceInventory, EItemType ItemType, FName SlotType, FName ComingFromSlotType)
{
	if(SourceInventory)
	{
		const FSlotStruct SlotContent = SourceInventory->Content[SourceIndex];
		
		switch (ItemType)
		{
		case EItemType::EIT_Weapon:
			if (SlotType == FName("Weapon"))
			{
				if(ComingFromSlotType == FName("None"))
				{
					SourceInventory->Content[SourceIndex] = PrimaryWeaponSlot;
					PrimaryWeaponSlot = SlotContent;
				}
			}
			if (SlotType == FName("None"))
			{
				if(ComingFromSlotType == FName("Weapon"))
				{
					FSlotStruct TempSlot = SourceInventory->Content[DestinationIndex];
					SourceInventory->Content[DestinationIndex] = PrimaryWeaponSlot;
					PrimaryWeaponSlot = TempSlot;
				}
			}
			break;
		case EItemType::EIT_Shield:
			if (SlotType == FName("Shield"))
			{
				if(ComingFromSlotType == FName("None"))
				{
					SourceInventory->Content[SourceIndex] = ShieldSlot;
					ShieldSlot = SlotContent;
				}
			}
			if (SlotType == FName("None"))
			{
				FSlotStruct TempSlot = SourceInventory->Content[DestinationIndex];
				SourceInventory->Content[DestinationIndex] = ShieldSlot;
				ShieldSlot = TempSlot;
			}
		case EItemType::EIT_Unequippable:
			
			break;
		}
		MulticastUpdateInventory();
		SourceInventory->MulticastUpdateInventory();
		if(Character) Character->TryEquipWeapons(PrimaryWeaponSlot, ShieldSlot);
	}
}

void UInventoryComponent::RemoveFromInventory(int32 Index, bool bRemoveWholeStack, bool bIsConsumed)
{
	FName Item = Content[Index].ItemID;
	int32 Quantity = Content[Index].Quantity;
	
	if(bRemoveWholeStack || Quantity == 1)
	{
		const FSlotStruct EmptyStruct;
		Content[Index] = EmptyStruct;
		if(bIsConsumed)
		{
			// TODO: This is DISGUSTING
			if(IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(GetOwner()))
			{
				if(const USGAttributeSet* SGAttributeSet = Cast<USGAttributeSet>(ASCInterface->GetAbilitySystemComponent()->GetAttributeSet(USGAttributeSet::StaticClass())))
				{
					USGAttributeSet* MutableSGAttributeSet = const_cast<USGAttributeSet*>(SGAttributeSet);
					MutableSGAttributeSet->SetHealth(SGAttributeSet->GetHealth() + 25.f);
				}
			}
		}
		else
		{
			ServerDropItem(Item, Quantity);
		}
	}
	else
	{
		Content[Index].Quantity -= 1;
		if(bIsConsumed)
		{
			// TODO: This is DISGUSTING
			if(IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(GetOwner()))
			{
				if(const USGAttributeSet* SGAttributeSet = Cast<USGAttributeSet>(ASCInterface->GetAbilitySystemComponent()->GetAttributeSet(USGAttributeSet::StaticClass())))
				{
					USGAttributeSet* MutableSGAttributeSet = const_cast<USGAttributeSet*>(SGAttributeSet);
					MutableSGAttributeSet->SetHealth(SGAttributeSet->GetHealth() + 25.f);
				}
			}
		}
		else
		{
			ServerDropItem(Item, 1);
		}
	}
	MulticastUpdateInventory();
}

void UInventoryComponent::ServerDropItem_Implementation(FName ItemID, int32 Quantity)
{
	for (int32 i = 0; i < Quantity; ++i)
	{
		FItemStruct ItemToSpawn = GetItemData(ItemID);
		if(ItemToSpawn.ItemClass)
		{
			FActorSpawnParameters SpawnParameters;
			GetWorld()->SpawnActor<AActor>(ItemToSpawn.ItemClass, GetDropLocation(), FRotator(180.f, 180.f, 90.f), SpawnParameters);
		}
	}
}

void UInventoryComponent::ServerRemove_Implementation(int32 Index, bool bRemoveWholeStack, bool bIsConsumed)
{
	RemoveFromInventory(Index, bRemoveWholeStack, bIsConsumed);
}

FItemStruct UInventoryComponent::GetItemData(FName ItemID) const
{
	if(DataTable)
	{
		static const FString ContextString(TEXT("Item Data Context"));
		if(const FItemStruct* ItemData = DataTable->FindRow<FItemStruct>(ItemID, ContextString, true))
		{
			return *ItemData;
		}
		return FItemStruct();
	}
	return FItemStruct();
}

void UInventoryComponent::ClientOnLocalInteract_Implementation(AActor* TargetActor, AActor* Interactor)
{
	if(IInteractInterface* InteractInterface = Cast<IInteractInterface>(TargetActor))
	{
		if(ASGCharacter* InteractingSGCharacter = Cast<ASGCharacter>(Interactor))
		{
			InteractInterface->InteractWith(InteractingSGCharacter);
		}
	}
}

void UInventoryComponent::ServerTransferSlots_Implementation(int32 SourceIndex, UInventoryComponent* SourceInventory, int32 DestinationIndex)
{
	TransferSlots(SourceIndex, SourceInventory, DestinationIndex);
}

void UInventoryComponent::ServerTransferEquippableSlots_Implementation(int32 SourceIndex, int32 DestinationIndex,
	UInventoryComponent* SourceInventory, EItemType ItemType, FName SlotType, FName ComingFromSlotType)
{
	TransferEquippableSlots(SourceIndex, DestinationIndex, SourceInventory, ItemType, SlotType, ComingFromSlotType);
}

void UInventoryComponent::MulticastUpdateInventory_Implementation()
{
	OnInventoryUpdated.Broadcast();
}

FVector UInventoryComponent::GetDropLocation() const
{
	FHitResult OutResult;
	
	const FVector OwnerLocation = GetOwner()->GetActorLocation();
	constexpr float RandomRadius = 50.0f;
	const FVector RandomOffset = UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(0.f, RandomRadius);
	const FVector StartLocation = OwnerLocation + FVector(0.f, 0.f, 50.f) + RandomOffset;
	const FVector EndLocation = StartLocation - FVector(0.f, 0.f, 500.f);

	GetWorld()->LineTraceSingleByChannel(OutResult, StartLocation, EndLocation, ECC_Visibility);

	if (OutResult.bBlockingHit)
	{
		OutResult.Location.Z += 5.f;
		return OutResult.Location;
	}
	
	return OwnerLocation + RandomOffset;
}



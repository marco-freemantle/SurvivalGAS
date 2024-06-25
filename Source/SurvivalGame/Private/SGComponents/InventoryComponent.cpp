// Copyright Marco Freemantle


#include "SGComponents/InventoryComponent.h"

#include "Character/SGCharacter.h"
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
		if (IInteractInterface* InteractInterface = Cast<IInteractInterface>(Target))
		{
			if (ASGCharacter* SGCharacter = Cast<ASGCharacter>(GetOwner()))
			{
				Target->SetOwner(SGCharacter->GetController());

				ClientOnLocalInteract(Target, SGCharacter);
			}
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
	int32 AvailableIndex = AnyEmptySlotsAvailable();
	if(AvailableIndex >= 0)
	{
		FSlotStruct NewSlotStruct;
		NewSlotStruct.ItemID = ItemID;
		NewSlotStruct.Quantity = 1;
		Content[AvailableIndex] = NewSlotStruct;
		return true;
	}
	return false;
}

// Only called if dropping items between different container
void UInventoryComponent::TransferSlots(int32 SourceIndex, UInventoryComponent* SourceInventory, int32 DestinationIndex)
{
	if(SourceInventory)
	{
		const FSlotStruct SlotContent = SourceInventory->Content[SourceIndex];

		if(DestinationIndex < 0)
		{
			
		}
		else
		{
			// GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.f, FColor::Red, FString("Here"));
			// Items are the same type -> try stack
			if(Content[DestinationIndex].ItemID == SlotContent.ItemID)
			{
				int32 RemainingItems = FMath::Clamp((Content[DestinationIndex].Quantity + SlotContent.Quantity) - GetMaxStackSize(SlotContent.ItemID), 0, GetMaxStackSize(SlotContent.ItemID));
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
			}
			else
			{
				SourceInventory->Content[SourceIndex] = Content[DestinationIndex];
				Content[DestinationIndex] = SlotContent;

				MulticastUpdateInventory();
				SourceInventory->MulticastUpdateInventory();
			}
		}
	}
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

void UInventoryComponent::MulticastUpdateInventory_Implementation()
{
	OnInventoryUpdated.Broadcast();
}



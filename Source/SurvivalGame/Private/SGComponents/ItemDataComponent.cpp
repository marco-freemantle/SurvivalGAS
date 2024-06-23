// Copyright Marco Freemantle


#include "SGComponents/ItemDataComponent.h"

#include "Character/SGCharacter.h"
#include "SGComponents/InventoryComponent.h"

UItemDataComponent::UItemDataComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UItemDataComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UItemDataComponent::InteractWith(ASGCharacter* SGCharacter)
{
	if(SGCharacter && SGCharacter->GetInventoryComponent())
	{
		SGCharacter->GetInventoryComponent()->AddToInventory(ItemID.RowName, Quantity);
		GetOwner()->Destroy();
	}
}


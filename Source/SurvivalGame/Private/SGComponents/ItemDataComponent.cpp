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
		SGCharacter->ClientPlayPickupSound();
		GetOwner()->Destroy();
	}
}

void UItemDataComponent::ShowPickupWidget(bool bShowWidget)
{
	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 4.f, FColor::Red, FString("2"));

}


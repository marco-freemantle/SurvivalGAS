// Copyright Marco Freemantle

#include "HUD/SGHUD.h"
#include "SGComponents/InventoryComponent.h"
#include "Blueprint/UserWidget.h"

void ASGHUD::DrawHUD()
{
	Super::DrawHUD();
}

void ASGHUD::BeginPlay()
{
	Super::BeginPlay();
}

void ASGHUD::AddCharacterSheet()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if(PlayerController && CharacterSheetClass)
	{
		CharacterSheet = CreateWidget<UUserWidget>(PlayerController, CharacterSheetClass);
		CharacterSheet->AddToViewport();
		const FInputModeGameAndUI InputMode;
		PlayerController->SetInputMode(InputMode);
		PlayerController->SetShowMouseCursor(true);
	}
}

void ASGHUD::RemoveCharacterSheet() const
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if(PlayerController && CharacterSheet)
	{
		CharacterSheet->RemoveFromParent();
		const FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);
		PlayerController->SetShowMouseCursor(false);
	}
}

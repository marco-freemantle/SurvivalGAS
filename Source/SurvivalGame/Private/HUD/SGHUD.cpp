// Copyright Marco Freemantle

#include "HUD/SGHUD.h"
#include "Blueprint/UserWidget.h"
#include "Player/SGPlayerController.h"

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
	ASGPlayerController* PlayerController = Cast<ASGPlayerController>(GetOwningPlayerController());
	if(PlayerController && CharacterSheetClass && !bIsContainerOpen)
	{
		CharacterSheet = CreateWidget<UUserWidget>(PlayerController, CharacterSheetClass);
		CharacterSheet->AddToViewport();
		const FInputModeGameAndUI InputMode;
		PlayerController->SetInputMode(InputMode);
		PlayerController->SetShowMouseCursor(true);
		PlayerController->SetIsCharacterSheetOpen(true);
		PlayerController->PlayOpenInventorySound();
	}
}

void ASGHUD::RemoveCharacterSheet() const
{
	ASGPlayerController* PlayerController = Cast<ASGPlayerController>(GetOwningPlayerController());
	if(PlayerController && CharacterSheet)
	{
		CharacterSheet->RemoveFromParent();
		const FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);
		PlayerController->SetShowMouseCursor(false);
		PlayerController->SetIsCharacterSheetOpen(false);
		PlayerController->PlayCloseInventorySound();
	}
}

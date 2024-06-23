// Copyright Marco Freemantle


#include "HUD/SGHUD.h"

#include "Blueprint/UserWidget.h"

void ASGHUD::DrawHUD()
{
	Super::DrawHUD();
}

void ASGHUD::BeginPlay()
{
	Super::BeginPlay();

	AddCharacterSheet();
}

void ASGHUD::AddCharacterSheet()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if(PlayerController && CharacterSheetClass)
	{
		CharacterSheet = CreateWidget<UUserWidget>(PlayerController, CharacterSheetClass);
		CharacterSheet->AddToViewport();
		CharacterSheet->SetVisibility(ESlateVisibility::Hidden);
	}
}

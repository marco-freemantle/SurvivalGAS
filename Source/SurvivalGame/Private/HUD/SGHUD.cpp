// Copyright Marco Freemantle

#include "HUD/SGHUD.h"
#include "HUD/Widget/SGUserWidget.h"
#include "HUD/WidgetController/OverlayWidgetController.h"
#include "Player/SGPlayerController.h"

void ASGHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("Overlay Widget Class uninitialised"));
	checkf(OverlayWidgetControllerClass, TEXT("Overlay Widget Controller Class uninitialised"));
	
	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	OverlayWidget = Cast<USGUserWidget>(Widget);

	const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
	UOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);

	OverlayWidget->SetWidgetController(WidgetController);
	WidgetController->BroadcastInitialValues();
	Widget->AddToViewport();
}

UOverlayWidgetController* ASGHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	if(OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(WCParams);
		OverlayWidgetController->BindCallbacksToDependencies();

		return OverlayWidgetController;
	}
	return OverlayWidgetController;
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

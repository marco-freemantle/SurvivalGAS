// Copyright Marco Freemantle

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SGHUD.generated.h"

class UAttributeSet;
class UAbilitySystemComponent;
struct FWidgetControllerParams;
class UOverlayWidgetController;
class USGUserWidget;
class UInventoryComponent;
/**
 * 
 */
UCLASS()
class SURVIVALGAME_API ASGHUD : public AHUD
{
	GENERATED_BODY()

public:
	void InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);
	
	UOverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerParams& WCParams);
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<USGUserWidget> OverlayWidgetClass;

	UPROPERTY()
	TObjectPtr<USGUserWidget> OverlayWidget;

	UPROPERTY(EditAnywhere, Category="Player Stats")
	TSubclassOf<UUserWidget> CharacterSheetClass;

	UPROPERTY()
	UUserWidget* CharacterSheet;

	void AddCharacterSheet();

	UFUNCTION(BlueprintCallable)
	void RemoveCharacterSheet() const;

	UFUNCTION(BlueprintImplementableEvent)
	void ToggleShowContainer(UInventoryComponent* ContainerInventory);

	UFUNCTION(BlueprintImplementableEvent)
	void HideContainer();

	UPROPERTY(BlueprintReadWrite)
	bool bIsContainerOpen = false;

private:
	UPROPERTY()
	UOverlayWidgetController* OverlayWidgetController;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UOverlayWidgetController> OverlayWidgetControllerClass;
};

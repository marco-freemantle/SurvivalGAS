// Copyright Marco Freemantle

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SGHUD.generated.h"

class UInventoryComponent;
/**
 * 
 */
UCLASS()
class SURVIVALGAME_API ASGHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

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

protected:
	virtual void BeginPlay() override;
};

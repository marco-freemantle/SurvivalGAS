// Copyright Marco Freemantle

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SGHUD.generated.h"

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

protected:
	virtual void BeginPlay() override;
};

// Copyright Marco Freemantle

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SGUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALGAME_API USGUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetWidgetController(UObject* InWidgetController);
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UObject> WidgetController;

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void WidgetControllerSet();
};

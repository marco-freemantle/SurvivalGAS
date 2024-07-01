// Copyright Marco Freemantle

#include "HUD/WidgetController/SGWidgetController.h"

void USGWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WCParams)
{
	PlayerController = WCParams.PlayerController;
	PlayerState = WCParams.PlayerState;
	AbilitySystemComponent = WCParams.AbilitySystemComponent;
	AttributeSet = WCParams.AttributeSet;
}

void USGWidgetController::BroadcastInitialValues()
{
}

void USGWidgetController::BindCallbacksToDependencies()
{
}

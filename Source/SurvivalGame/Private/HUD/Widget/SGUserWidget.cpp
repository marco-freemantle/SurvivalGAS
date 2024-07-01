// Copyright Marco Freemantle

#include "HUD/Widget/SGUserWidget.h"

void USGUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	WidgetControllerSet();
}

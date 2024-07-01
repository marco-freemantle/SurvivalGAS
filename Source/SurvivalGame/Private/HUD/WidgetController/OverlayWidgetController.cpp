// Copyright Marco Freemantle


#include "HUD/WidgetController/OverlayWidgetController.h"

#include "AbilitySystem/SGAttributeSet.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	const USGAttributeSet* SGAttributeSet = CastChecked<USGAttributeSet>(AttributeSet);

	OnHealthChanged.Broadcast(SGAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(SGAttributeSet->GetMaxHealth());
	
	OnManaChanged.Broadcast(SGAttributeSet->GetMana());
	OnMaxManaChanged.Broadcast(SGAttributeSet->GetMaxMana());
	
	OnStaminaChanged.Broadcast(SGAttributeSet->GetStamina());
	OnMaxStaminaChanged.Broadcast(SGAttributeSet->GetMaxStamina());
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	const USGAttributeSet* SGAttributeSet = CastChecked<USGAttributeSet>(AttributeSet);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(SGAttributeSet->GetHealthAttribute()).AddUObject(this, &ThisClass::HealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(SGAttributeSet->GetMaxHealthAttribute()).AddUObject(this, &ThisClass::MaxHealthChanged);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(SGAttributeSet->GetManaAttribute()).AddUObject(this, &ThisClass::ManaChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(SGAttributeSet->GetMaxManaAttribute()).AddUObject(this, &ThisClass::MaxManaChanged);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(SGAttributeSet->GetStaminaAttribute()).AddUObject(this, &ThisClass::StaminaChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(SGAttributeSet->GetMaxStaminaAttribute()).AddUObject(this, &ThisClass::MaxStaminaChanged);
}

void UOverlayWidgetController::HealthChanged(const FOnAttributeChangeData& Data) const
{
	OnHealthChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::MaxHealthChanged(const FOnAttributeChangeData& Data) const
{
	OnMaxHealthChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::ManaChanged(const FOnAttributeChangeData& Data) const
{
	OnManaChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::MaxManaChanged(const FOnAttributeChangeData& Data) const
{
	OnMaxManaChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::StaminaChanged(const FOnAttributeChangeData& Data) const
{
	OnStaminaChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::MaxStaminaChanged(const FOnAttributeChangeData& Data) const
{
	OnMaxStaminaChanged.Broadcast(Data.NewValue);
}

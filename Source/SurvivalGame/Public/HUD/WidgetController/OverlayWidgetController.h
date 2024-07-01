// Copyright Marco Freemantle

#pragma once

#include "CoreMinimal.h"
#include "HUD/WidgetController/SGWidgetController.h"
#include "OverlayWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedSignature, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealthChangedSignature, float, NewMaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnManaChangedSignature, float, NewMana);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxManaChangedSignature, float, NewMaxMana);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaminaChangedSignature, float, NewStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxStaminaChangedSignature, float, NewMaxStamina);

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class SURVIVALGAME_API UOverlayWidgetController : public USGWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	UPROPERTY(BlueprintAssignable, Category="GAS-Attributes")
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category="GAS-Attributes")
	FOnMaxHealthChangedSignature OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable, Category="GAS-Attributes")
	FOnManaChangedSignature OnManaChanged;

	UPROPERTY(BlueprintAssignable, Category="GAS-Attributes")
	FOnMaxManaChangedSignature OnMaxManaChanged;
	
	UPROPERTY(BlueprintAssignable, Category="GAS-Attributes")
	FOnStaminaChangedSignature OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category="GAS-Attributes")
	FOnMaxStaminaChangedSignature OnMaxStaminaChanged;

protected:
	void HealthChanged(const FOnAttributeChangeData& Data) const;
	void MaxHealthChanged(const FOnAttributeChangeData& Data) const;
	void ManaChanged(const FOnAttributeChangeData& Data) const;
	void MaxManaChanged(const FOnAttributeChangeData& Data) const;
	void StaminaChanged(const FOnAttributeChangeData& Data) const;
	void MaxStaminaChanged(const FOnAttributeChangeData& Data) const;
};

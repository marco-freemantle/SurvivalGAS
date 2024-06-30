// Copyright Marco Freemantle

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "SGEnemy.generated.h"

class UAttributeSet;
class UAbilitySystemComponent;

UCLASS()
class SURVIVALGAME_API ASGEnemy : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ASGEnemy();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

};

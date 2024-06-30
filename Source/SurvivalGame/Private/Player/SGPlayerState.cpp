// Copyright Marco Freemantle

#include "Player/SGPlayerState.h"
#include "AbilitySystem/SGAbilitySystemComponent.h"
#include "AbilitySystem/SGAttributeSet.h"

ASGPlayerState::ASGPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<USGAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	AttributeSet = CreateDefaultSubobject<USGAttributeSet>(TEXT("AttributeSet"));
	
	NetUpdateFrequency = 100.f;
}

UAbilitySystemComponent* ASGPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

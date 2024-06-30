// Copyright Marco Freemantle


#include "Character/SGEnemy.h"

#include "AbilitySystem/SGAbilitySystemComponent.h"
#include "AbilitySystem/SGAttributeSet.h"

ASGEnemy::ASGEnemy()
{
	AbilitySystemComponent = CreateDefaultSubobject<USGAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	AttributeSet = CreateDefaultSubobject<USGAttributeSet>(TEXT("AttributeSet"));

	NetUpdateFrequency = 100.f;
	MinNetUpdateFrequency = 33.f;
}

void ASGEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

UAbilitySystemComponent* ASGEnemy::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}



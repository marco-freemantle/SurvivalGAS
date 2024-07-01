// Copyright Marco Freemantle


#include "Actor/ConsumableActor.h"

#include "AbilitySystem/SGAttributeSet.h"
#include "Character/SGCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "SGComponents/ItemDataComponent.h"

AConsumableActor::AConsumableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InteractWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	InteractWidget->SetupAttachment(RootComponent);

	ItemDataComponent = CreateDefaultSubobject<UItemDataComponent>(TEXT("ItemDataComponent"));
	ItemDataComponent->SetIsReplicated(true);
}

void AConsumableActor::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AConsumableActor::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AConsumableActor::OnSphereEndOverlap);
		SetReplicateMovement(true);
	}
	if(InteractWidget)
	{
		InteractWidget->SetVisibility(false);
	}
}

void AConsumableActor::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(ASGCharacter* SGCharacter = Cast<ASGCharacter>(OtherActor))
	{
		SGCharacter->SetOverlappingInteractable(this);
	}
}

void AConsumableActor::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if(ASGCharacter* SGCharacter = Cast<ASGCharacter>(OtherActor))
	{
		SGCharacter->SetOverlappingInteractable(nullptr);
	}
}

void AConsumableActor::ShowPickupWidget(const bool bShowWidget)
{
	if(InteractWidget)
	{
		InteractWidget->SetVisibility(bShowWidget);
	}
}

void AConsumableActor::InteractWith(ASGCharacter* SGCharacter)
{
}

void AConsumableActor::ConsumeItem(AActor* TargetActor) const
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if(!TargetASC) return;

	check(GameplayEffectClass);
	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass, 1.f, EffectContextHandle);
	TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
}


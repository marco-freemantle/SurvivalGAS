// Copyright Marco Freemantle


#include "Actor/PickupActor.h"

#include "Character/SGCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "SGComponents/ItemDataComponent.h"

APickupActor::APickupActor()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
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

void APickupActor::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &APickupActor::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &APickupActor::OnSphereEndOverlap);
		SetReplicateMovement(true);
	}
	if(InteractWidget)
	{
		InteractWidget->SetVisibility(false);
	}
}

void APickupActor::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(ASGCharacter* SGCharacter = Cast<ASGCharacter>(OtherActor))
	{
		SGCharacter->SetOverlappingInteractable(this);
	}
}

void APickupActor::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if(ASGCharacter* SGCharacter = Cast<ASGCharacter>(OtherActor))
	{
		SGCharacter->SetOverlappingInteractable(nullptr);
		SGCharacter->ClientHideContainerWidget(InventoryComponent);
	}
}

void APickupActor::ShowPickupWidget(const bool bShowWidget)
{
	if(InteractWidget)
	{
		InteractWidget->SetVisibility(bShowWidget);
	}
	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 4.f, FColor::Red, FString("1"));
}

void APickupActor::InteractWith(ASGCharacter* SGCharacter)
{
}


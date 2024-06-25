// Copyright Marco Freemantle

#include "Actor/Chest.h"

#include "Character/SGCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "HUD/SGHUD.h"
#include "Player/SGPlayerController.h"
#include "SGComponents/InventoryComponent.h"

AChest::AChest()
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

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	InventoryComponent->SetIsReplicated(true);
}

void AChest::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AChest::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AChest::OnSphereEndOverlap);
		SetReplicateMovement(true);
	}
	if(InteractWidget)
	{
		InteractWidget->SetVisibility(false);
	}
}

void AChest::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(ASGCharacter* SGCharacter = Cast<ASGCharacter>(OtherActor))
	{
		SGCharacter->SetOverlappingInteractable(this);
	}
}

void AChest::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if(ASGCharacter* SGCharacter = Cast<ASGCharacter>(OtherActor))
	{
		SGCharacter->SetOverlappingInteractable(nullptr);
		SGCharacter->ClientHideContainerWidget(InventoryComponent);
	}
}

void AChest::ShowPickupWidget(const bool bShowWidget)
{
	if(InteractWidget)
	{
		InteractWidget->SetVisibility(bShowWidget);
	}
}

void AChest::InteractWith(ASGCharacter* SGCharacter)
{
	if(ASGPlayerController* SGController = Cast<ASGPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		if(ASGHUD* SGHUD = Cast<ASGHUD>(SGController->GetHUD()))
		{
			SGHUD->ToggleShowContainer(InventoryComponent);
		}
	}
}



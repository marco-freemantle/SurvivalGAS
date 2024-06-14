// Copyright Marco Freemantle

#include "Weapon/Weapon.h"
#include "Character/SGCharacter.h"
#include "Player/SGPlayerController.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
		WeaponMesh->OnComponentHit.AddDynamic(this, &AWeapon::OnHit);
		SetReplicateMovement(true);
	}
	
	if(PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
}

void AWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if(HasAuthority() && bIsTraceActive)
	{
		FVector StartSocket = WeaponMesh->GetSocketLocation(FName("Start"));
		FVector EndSocket = WeaponMesh->GetSocketLocation(FName("End"));

		TArray<FHitResult> OutResults;
		
		bool bHit = GetWorld()->SweepMultiByChannel(
		   OutResults,
		   StartSocket,
		   EndSocket,
		   FQuat::Identity,
		   ECC_Pawn,
		   FCollisionShape::MakeCapsule(10.f, 75.f)
	   );

		FVector Midpoint = (StartSocket + EndSocket) / 2.0f;
		FVector CapsuleAxis = (EndSocket - StartSocket).GetSafeNormal();
		FRotator CapsuleRotation = FRotationMatrix::MakeFromZ(CapsuleAxis).Rotator();

		DrawDebugCapsule(
			GetWorld(),
			Midpoint,
			75.f,
			10.f,
			CapsuleRotation.Quaternion(),
			FColor::Red,
			false, // Persistent lines (set to true if you want the lines to persist)
			5.0f,  // Duration the debug lines should be visible (set to 0 for infinite)
			0,     // Depth priority (default is 0)
			1.0f   // Line thickness
		);

		if (bHit)
		{
			for (const FHitResult& Hit : OutResults)
			{
				HitActors.AddUnique(Hit.GetActor());
			}
		}
	}
}

void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		OnEquipped();
		break;
	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondary();
		break;
	case EWeaponState::EWS_Dropped:
		OnDropped();
		break;
	}
}

void AWeapon::OnEquipped()
{
	SGOwnerCharacter = SGOwnerCharacter == nullptr ? Cast<ASGCharacter>(Owner) : SGOwnerCharacter;
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	bCanPlayHitFloorSound = true;
}

void AWeapon::OnDropped()
{
	if(HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AWeapon::OnEquippedSecondary()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if(Owner == nullptr)
	{
		SGOwnerCharacter = nullptr;
		SGOwnerCharacter = nullptr;
	}
	else
	{
		SGOwnerCharacter = SGOwnerCharacter == nullptr ? Cast<ASGCharacter>(Owner) : SGOwnerCharacter;
	}
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(ASGCharacter* SGCharacter = Cast<ASGCharacter>(OtherActor))
	{
		SGCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if(ASGCharacter* SGCharacter = Cast<ASGCharacter>(OtherActor))
	{
		SGCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if(bCanPlayHitFloorSound)
	{
		MulticastPlayHitFloorSound();
	}
	bCanPlayHitFloorSound = false;
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	OnWeaponStateSet();
}

void AWeapon::OnRep_WeaponState()
{
	OnWeaponStateSet();
}

void AWeapon::ShowPickupWidget(bool bShowWidget) const
{
	if(PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}


void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	if(SGOwnerCharacter && SGOwnerCharacter->IsLocallyControlled())
	{
		WeaponMesh->SetVisibility(true);
	}
	SetOwner(nullptr);
	SGOwnerCharacter = nullptr;
	SGOwnerController = nullptr;
}

void AWeapon::StartTraceAttack()
{
	HitActors.Empty();
	bIsTraceActive = true;
}

void AWeapon::EndTraceAttack()
{
	for(auto actor : HitActors)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.f, FColor::Red, FString::Printf(TEXT("%s"), *actor->GetName()));
	}
	bIsTraceActive = false;
}

void AWeapon::MulticastPlayHitFloorSound_Implementation()
{
	if(HitFloorSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitFloorSound, GetActorLocation());
	}
}


// Copyright Marco Freemantle

#include "Player/SGPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Character/SGCharacter.h"
#include "Game/SGGameUserSettings.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HUD/SGHUD.h"
#include "Input/SGInputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SGComponents/CombatComponent.h"
#include "SGComponents/LockonComponent.h"

ASGPlayerController::ASGPlayerController()
{
	bReplicates = true;
}

void ASGPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ASGPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
}

void ASGPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	check(SGContext);

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(SGContext, 0);
	}

	bShowMouseCursor = false;

	const FInputModeGameOnly InputModeData;
	SetInputMode(InputModeData);
}

void ASGPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void ASGPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	USGInputComponent* SGInputComponent = CastChecked<USGInputComponent>(InputComponent);

	SGInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASGPlayerController::Move);
	SGInputComponent->BindAction(LookUpAction, ETriggerEvent::Triggered, this, &ASGPlayerController::LookUp);
	SGInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &ASGPlayerController::Turn);
	SGInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ASGPlayerController::Jump);
	SGInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ASGPlayerController::Interact);
	SGInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &ASGPlayerController::SetbCanEquipTrue);
	SGInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ASGPlayerController::Crouch);
	SGInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ASGPlayerController::UnCrouch);
	SGInputComponent->BindAction(PauseGameAction, ETriggerEvent::Started, this, &ASGPlayerController::PauseGame);
	SGInputComponent->BindAction(LockonAction, ETriggerEvent::Started, this, &ASGPlayerController::Lockon);
	SGInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ASGPlayerController::Attack);
	SGInputComponent->BindAction(BlockAction, ETriggerEvent::Started, this, &ASGPlayerController::Block);
	SGInputComponent->BindAction(BlockAction, ETriggerEvent::Completed, this, &ASGPlayerController::Unblock);
	SGInputComponent->BindAction(SwitchLockonTargetLeftAction, ETriggerEvent::Started, this, &ASGPlayerController::SwitchLockonTargetLeft);
	SGInputComponent->BindAction(SwitchLockonTargetRightAction, ETriggerEvent::Started, this, &ASGPlayerController::SwitchLockonTargetRight);
	SGInputComponent->BindAction(DrawPrimaryAction, ETriggerEvent::Started, this, &ASGPlayerController::DrawPrimary);
	SGInputComponent->BindAction(ToggleCharacterSheetAction, ETriggerEvent::Started, this, &ASGPlayerController::ToggleCharacterSheet);
}

void ASGPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

void ASGPlayerController::LookUp(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		if(USGGameUserSettings* UserSettings = Cast<USGGameUserSettings>(UGameUserSettings::GetGameUserSettings()))
		{
			ControlledPawn->AddControllerPitchInput(InputAxisVector.X * UserSettings->GetMouseSensitivity());
		}
	}
}

void ASGPlayerController::Turn(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		if(USGGameUserSettings* UserSettings = Cast<USGGameUserSettings>(UGameUserSettings::GetGameUserSettings()))
		{
			ControlledPawn->AddControllerYawInput(InputAxisVector.X * UserSettings->GetMouseSensitivity());
		}
	}
}

void ASGPlayerController::Jump(const FInputActionValue& InputActionValue)
{
	if (ASGCharacter* SGCharacter = Cast<ASGCharacter>(GetCharacter()))
	{
		if(SGCharacter->GetCombatComponent() && SGCharacter->GetCombatComponent()->GetCombatState() == ECombatState::ECS_Unoccupied)
		SGCharacter->Jump();
	}
}

void ASGPlayerController::Interact(const FInputActionValue& InputActionValue)
{
	if(!bCanEquip) return;
	if (ASGCharacter* SGCharacter = Cast<ASGCharacter>(GetCharacter()))
	{
		SGCharacter->InteractButtonPressed();
	}
	bCanEquip = false;
}

void ASGPlayerController::Attack(const FInputActionValue& InputActionValue)
{
	if (ASGCharacter* SGCharacter = Cast<ASGCharacter>(GetCharacter()))
	{
		SGCharacter->AttackButtonPressed();
	}
}

void ASGPlayerController::Block(const FInputActionValue& InputActionValue)
{
	if (ASGCharacter* SGCharacter = Cast<ASGCharacter>(GetCharacter()))
	{
		SGCharacter->BlockButtonPressed();
	}
}

void ASGPlayerController::Unblock(const FInputActionValue& InputActionValue)
{
	if (ASGCharacter* SGCharacter = Cast<ASGCharacter>(GetCharacter()))
	{
		SGCharacter->BlockButtonReleased();
	}
}

void ASGPlayerController::ToggleCharacterSheet(const FInputActionValue& InputActionValue)
{
	SGHUD = SGHUD == nullptr ? Cast<ASGHUD>(GetHUD()) : SGHUD;

	if (SGHUD)
	{
		if (!bIsCharacterSheetOpen)
		{
			SGHUD->AddCharacterSheet();
		}
		else
		{
			SGHUD->RemoveCharacterSheet();
		}
	}
}

void ASGPlayerController::Crouch(const FInputActionValue& InputActionValue)
{
	if (ASGCharacter* SGCharacter = Cast<ASGCharacter>(GetCharacter()))
	{
		if(SGCharacter->GetCharacterMovement()->IsFalling()) return;
		SGCharacter->Crouch();
	}
}

void ASGPlayerController::UnCrouch(const FInputActionValue& InputActionValue)
{
	if (ASGCharacter* SGCharacter = Cast<ASGCharacter>(GetCharacter()))
	{
		SGCharacter->UnCrouch();
	}
}

void ASGPlayerController::DrawPrimary(const FInputActionValue& InputActionValue)
{
	if (ASGCharacter* SGCharacter = Cast<ASGCharacter>(GetCharacter()))
	{
		SGCharacter->DrawPrimaryButtonPressed();
	}
}

void ASGPlayerController::Lockon(const FInputActionValue& InputActionValue)
{
	if (ASGCharacter* SGCharacter = Cast<ASGCharacter>(GetCharacter()))
	{
		if(SGCharacter->GetLockonComponent() && SGCharacter->GetLockonComponent()->bIsLockedOnTarget)
		{
			SGCharacter->GetLockonComponent()->DisengageLockon();
		}
		else
		{
			SGCharacter->GetLockonComponent()->EngageLockon();
		}
	}
}

void ASGPlayerController::SwitchLockonTargetLeft(const FInputActionValue& InputActionValue)
{
	if (const ASGCharacter* SGCharacter = Cast<ASGCharacter>(GetCharacter()))
	{
		if(SGCharacter->GetLockonComponent() && SGCharacter->GetLockonComponent()->bIsLockedOnTarget)
		{
			SGCharacter->GetLockonComponent()->SwitchLockonTargetLeft();
		}
	}
}

void ASGPlayerController::SwitchLockonTargetRight(const FInputActionValue& InputActionValue)
{
	if (const ASGCharacter* SGCharacter = Cast<ASGCharacter>(GetCharacter()))
	{
		if(SGCharacter->GetLockonComponent() && SGCharacter->GetLockonComponent()->bIsLockedOnTarget)
		{
			SGCharacter->GetLockonComponent()->SwitchLockonTargetRight();
		}
	}
}

void ASGPlayerController::PauseGame(const FInputActionValue& InputActionValue)
{
	
}

void ASGPlayerController::PlayOpenInventorySound()
{
	if(OpenInventorySound)
	{
		UGameplayStatics::PlaySound2D(this, OpenInventorySound);
	}
}

void ASGPlayerController::PlayCloseInventorySound()
{
	if(CloseInventorySound)
	{
		UGameplayStatics::PlaySound2D(this, CloseInventorySound);
	}
}

void ASGPlayerController::SetbCanEquipTrue()
{
	bCanEquip = true;
}

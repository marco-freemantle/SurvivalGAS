// Copyright Marco Freemantle

#include "Player/SGPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Character/SGCharacter.h"
#include "Game/SGGameUserSettings.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Input/SGInputComponent.h"

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

	FInputModeGameOnly InputModeData;
	SetInputMode(InputModeData);
}

void ASGPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if(ASGCharacter* SGCharacter = Cast<ASGCharacter>(InPawn))
	{
		
	}
}

void ASGPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	USGInputComponent* VBInputComponent = CastChecked<USGInputComponent>(InputComponent);

	VBInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASGPlayerController::Move);
	VBInputComponent->BindAction(LookUpAction, ETriggerEvent::Triggered, this, &ASGPlayerController::LookUp);
	VBInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &ASGPlayerController::Turn);
	VBInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ASGPlayerController::Jump);
	VBInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &ASGPlayerController::Equip);
	VBInputComponent->BindAction(EquipAction, ETriggerEvent::Completed, this, &ASGPlayerController::SetbCanEquipTrue);
	VBInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ASGPlayerController::Crouch);
	VBInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ASGPlayerController::UnCrouch);
	VBInputComponent->BindAction(PauseGameAction, ETriggerEvent::Started, this, &ASGPlayerController::PauseGame);
	VBInputComponent->BindAction(SwapWeaponsAction, ETriggerEvent::Started, this, &ASGPlayerController::SwapWeapons);
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
		SGCharacter->Jump();
	}
}

void ASGPlayerController::Equip(const FInputActionValue& InputActionValue)
{
	if(!bCanEquip) return;
	if (ASGCharacter* SGCharacter = Cast<ASGCharacter>(GetCharacter()))
	{
		
	}
	bCanEquip = false;
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

void ASGPlayerController::SwapWeapons(const FInputActionValue& InputActionValue)
{
	if (ASGCharacter* SGCharacter = Cast<ASGCharacter>(GetCharacter()))
	{
		
	}
}

void ASGPlayerController::PauseGame(const FInputActionValue& InputActionValue)
{
	
}

void ASGPlayerController::SetbCanEquipTrue()
{
	bCanEquip = true;
}

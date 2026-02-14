// Copyright Epic Games, Inc. All Rights Reserved.

#include "DroneRPGPlayerController.h"
#include "Engine/World.h"
#include <Kismet/KismetMathLibrary.h>

#include "DroneRPG/DroneRPGCharacter.h"
#include "DroneRPG/Components/HealthComponent.h"
#include "DroneRPG/GameModes/DroneRPGGameMode.h"
#include "DroneRPG/Utilities/FunctionLibrary.h"
#include "DroneRPG/Utilities/WeaponCreator.h"
#include "DroneRPG/Weapons/Weapon.h"
#include "Kismet/GameplayStatics.h"

#define mActorLocation GetCharacter()->GetActorLocation()
#define mActorRotation GetCharacter()->GetActorRotation()

void ADroneRPGPlayerController::One()
{
	SetWeapon(static_cast<EWeaponType>(1));
}

void ADroneRPGPlayerController::Two()
{
	SetWeapon(static_cast<EWeaponType>(2));
}

void ADroneRPGPlayerController::Three()
{
	SetWeapon(static_cast<EWeaponType>(3));
}

void ADroneRPGPlayerController::Four()
{
	SetWeapon(static_cast<EWeaponType>(4));
}

void ADroneRPGPlayerController::Five()
{
	SetWeapon(static_cast<EWeaponType>(5));
}

void ADroneRPGPlayerController::SetWeapon(EWeaponType type)
{
	GetDrone()->SetWeapon(mGetWeapon(type, 0.3f, 0.3f, GetDrone()->GetCombatantComponent()));
}

const FName ADroneRPGPlayerController::MoveForwardBinding("MoveForward");
const FName ADroneRPGPlayerController::MoveRightBinding("MoveRight");
const FName ADroneRPGPlayerController::FireForwardBinding("FireForward");
const FName ADroneRPGPlayerController::FireRightBinding("FireRight");

ADroneRPGPlayerController::ADroneRPGPlayerController() : droneIndex(0), isFiring(false)
{
#if WITH_EDITOR
	SetFolderPath(TEXT("Other/Controllers"));
#endif
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
	MoveSpeed = 800.0f;
	moveCamera = true;
}

ADroneRPGCharacter* ADroneRPGPlayerController::GetDrone() const
{
	return droneCharacter;
}

void ADroneRPGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(GetGameMode()))
	{
		ChangeView();
	}
}

void ADroneRPGPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (IsValid(GetDrone()) && GetDrone()->GetHealthComponent()->IsAlive())
	{
		CalculateMovement(DeltaTime);

		FHitResult Hit;
		GetHitResultUnderCursor(ECC_WorldStatic, false, Hit);
		FRotator lookAt = UKismetMathLibrary::FindLookAtRotation(mActorLocation, Hit.ImpactPoint);
		lookAt.Pitch = mActorRotation.Pitch;
		lookAt.Roll = mActorRotation.Roll;

		GetDrone()->SetActorRotation(lookAt);

		// Try and fire a shot
		FireShot(mActorRotation.Vector());
	}
}

void ADroneRPGPlayerController::FireShot(const FVector& FireDirection)
{
	// If it's ok to fire again
	if (isFiring)
	{
		FHitResult Hit;
		GetHitResultUnderCursor(ECC_WorldStatic, false, Hit);
		GetDrone()->GetWeapon()->FireShot(FireDirection);
	}
}

void ADroneRPGPlayerController::UseTool()
{
	isFiring = true;
}

void ADroneRPGPlayerController::StopUsingTool()
{
	isFiring = false;
}

void ADroneRPGPlayerController::CalculateMovement(float DeltaSeconds) const
{
	// Find movement direction
	const float ForwardValue = GetInputAxisValue(MoveForwardBinding); // W S
	const float RightValue = GetInputAxisValue(MoveRightBinding); // A D

	// Clamp max size so that (X=1, Y=1) doesn't cause faster movement in diagonal directions
	const FVector MoveDirection = FVector(ForwardValue, RightValue, 0.f).GetClampedToMaxSize(1.0f);

	// Calculate  movement
	const FVector Movement = MoveDirection * MoveSpeed * DeltaSeconds;

	// If non-zero size, move this actor
	if (Movement.SizeSquared() > 0.0f)
	{
		GetDrone()->AddMovementInput(FRotationMatrix(GetControlRotation()).GetScaledAxis(EAxis::X), ForwardValue);
		GetDrone()->AddMovementInput(FRotationMatrix(GetControlRotation()).GetScaledAxis(EAxis::Y), RightValue);
	}
}

void ADroneRPGPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	InputComponent->BindAction("UseTool", IE_Pressed, this, &ADroneRPGPlayerController::UseTool);
	InputComponent->BindAction("UseTool", IE_Released, this, &ADroneRPGPlayerController::StopUsingTool);

	InputComponent->BindAction("One", IE_Pressed, this, &ADroneRPGPlayerController::One);
	InputComponent->BindAction("Two", IE_Pressed, this, &ADroneRPGPlayerController::Two);
	InputComponent->BindAction("Three", IE_Pressed, this, &ADroneRPGPlayerController::Three);
	InputComponent->BindAction("Four", IE_Pressed, this, &ADroneRPGPlayerController::Four);
	InputComponent->BindAction("Five", IE_Pressed, this, &ADroneRPGPlayerController::Five);

	// set up game play key bindings
	InputComponent->BindAxis(MoveForwardBinding);
	InputComponent->BindAxis(MoveRightBinding);
	InputComponent->BindAxis(FireForwardBinding);
	InputComponent->BindAxis(FireRightBinding);
}

ADroneRPGGameMode* ADroneRPGPlayerController::GetGameMode()
{
	if (!IsValid(gameMode))
	{
		gameMode = Cast<ADroneRPGGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	}
	return gameMode;
}

void ADroneRPGPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	droneCharacter = Cast<ADroneRPGCharacter>(aPawn);

	if (moveCamera)
	{
		ChangeView();
	}
}

void ADroneRPGPlayerController::ChangeView()
{
	UCombatantComponent* combatantFound = NULL;

	for (auto combatant : GetGameMode()->GetCombatants())
	{

		if (combatant != GetDrone()->GetCombatantComponent()) 
		{
			float combatScore = combatant->GetCombatScore();

			if (!IsValid(combatantFound))
			{
				combatantFound = combatant;
			}
			else if (combatScore > combatantFound->GetCombatScore())
			{
				combatantFound = combatant;
			}
		}
	}

	if (IsValid(combatantFound))
	{
		SetViewTargetWithBlend(combatantFound->GetOwner(), 1.0f, VTBlend_EaseInOut, 1, false);
		mSetTimer(TimerHandle_CameraTimer, &ADroneRPGPlayerController::ChangeView, 3.0f);
	}
}

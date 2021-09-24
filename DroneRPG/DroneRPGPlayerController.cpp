// Copyright Epic Games, Inc. All Rights Reserved.

#include "DroneRPGPlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "DroneRPGCharacter.h"
#include "Engine/World.h"
#include <Kismet/GameplayStatics.h>
#include <Kismet/KismetMathLibrary.h>
#include "DroneProjectile.h"
#include "FunctionLibrary.h"
#include "Weapon.h"

#define mActorLocation GetCharacter()->GetActorLocation()
#define mActorRotation GetCharacter()->GetActorRotation()

void ADroneRPGPlayerController::One()
{
	SetWeapon((EWeaponType)1);
}

void ADroneRPGPlayerController::Two()
{
	SetWeapon((EWeaponType)2);
}

void ADroneRPGPlayerController::Three()
{
	SetWeapon((EWeaponType)3);
}

void ADroneRPGPlayerController::Four()
{
	SetWeapon((EWeaponType)4);
}

void ADroneRPGPlayerController::Five()
{
	SetWeapon((EWeaponType)5);
}

void ADroneRPGPlayerController::SetWeapon(EWeaponType type) {
	GetDrone()->SetWeapon(mGetWeapon(type, 0.3f, 0.3f, GetDrone()));
}

const FName ADroneRPGPlayerController::MoveForwardBinding("MoveForward");
const FName ADroneRPGPlayerController::MoveRightBinding("MoveRight");
const FName ADroneRPGPlayerController::FireForwardBinding("FireForward");
const FName ADroneRPGPlayerController::FireRightBinding("FireRight");

ADroneRPGPlayerController::ADroneRPGPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
	MoveSpeed = 800.0f;
	moveCamera = true;
}

ADroneRPGCharacter* ADroneRPGPlayerController::GetDrone() {
	return Cast<ADroneRPGCharacter>(GetCharacter());
}

void ADroneRPGPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (moveCamera) {
		ChangeView();
	}

	if (GetCharacter() != NULL && GetDrone()->IsAlive()) {
		CalculateMovement(DeltaTime);

		// Create fire direction vector
		const float FireForwardValue = GetInputAxisValue(FireForwardBinding);
		const float FireRightValue = GetInputAxisValue(FireRightBinding);
		const FVector FireDirection = FVector(FireForwardValue, FireRightValue, 0.f);

		FHitResult Hit;
		GetHitResultUnderCursor(ECC_WorldStatic, false, Hit);
		FRotator lookAt = UKismetMathLibrary::FindLookAtRotation(mActorLocation, Hit.ImpactPoint);
		lookAt.Pitch = mActorRotation.Pitch;
		lookAt.Roll = mActorRotation.Roll;

		//GetCharacter()->SetActorRotation(FMath::Lerp(mActorRotation, lookAt, 0.10f));
		GetCharacter()->SetActorRotation(lookAt);

		// Try and fire a shot
		FireShot(mActorRotation.Vector());
	}
}

void ADroneRPGPlayerController::FireShot(FVector FireDirection)
{
	// If it's ok to fire again
	if (isFiring)
	{
		FHitResult Hit;
		GetHitResultUnderCursor(ECC_WorldStatic, false, Hit);
		ADroneRPGCharacter* target = mGetClosestEnemyInRadius(2000, Hit.ImpactPoint, GetDrone()->GetTeam());
		GetDrone()->GetWeapon()->FireShot(FireDirection, target);
	}
}

void ADroneRPGPlayerController::UseTool() {
	isFiring = true;
}

void ADroneRPGPlayerController::StopUsingTool() {
	isFiring = false;
}

void ADroneRPGPlayerController::CalculateMovement(float DeltaSeconds)
{
	// Find movement direction
	const float ForwardValue = GetInputAxisValue(MoveForwardBinding);
	const float RightValue = GetInputAxisValue(MoveRightBinding);

	// Clamp max size so that (X=1, Y=1) doesn't cause faster movement in diagonal directions
	const FVector MoveDirection = FVector(ForwardValue, RightValue, 0.f).GetClampedToMaxSize(1.0f);

	// Calculate  movement
	const FVector Movement = MoveDirection * MoveSpeed * DeltaSeconds;

	// If non-zero size, move this actor
	if (Movement.SizeSquared() > 0.0f)
	{
		GetCharacter()->AddMovementInput(FRotationMatrix(GetControlRotation()).GetScaledAxis(EAxis::X), ForwardValue);
		GetCharacter()->AddMovementInput(FRotationMatrix(GetControlRotation()).GetScaledAxis(EAxis::Y), RightValue);
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

void ADroneRPGPlayerController::CanMoveCamera() {
	moveCamera = true;
}

void ADroneRPGPlayerController::IncrementDrone() {
	droneIndex++;

	if (droneIndex > mGetDrones.Num() - 1)
		droneIndex = 0;
}

void ADroneRPGPlayerController::ChangeView()
{
	moveCamera = false;

	ADroneRPGCharacter* drone = mGetDrones[droneIndex];

	if (drone->GetController() == this) {
		IncrementDrone();
	}

	SetViewTargetWithBlend(drone, 1.0f, EViewTargetBlendFunction::VTBlend_EaseInOut, 1, false);

	IncrementDrone();

	mSetTimer(TimerHandle_CameraTimer, &ADroneRPGPlayerController::CanMoveCamera, 8.0f);
}

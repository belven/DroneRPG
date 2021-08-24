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

#define mActorLocation GetCharacter()->GetActorLocation()
#define mActorRotation GetCharacter()->GetActorRotation()

const FName ADroneRPGPlayerController::MoveForwardBinding("MoveForward");
const FName ADroneRPGPlayerController::MoveRightBinding("MoveRight");
const FName ADroneRPGPlayerController::FireForwardBinding("FireForward");
const FName ADroneRPGPlayerController::FireRightBinding("FireRight");

ADroneRPGPlayerController::ADroneRPGPlayerController()
{
	static ConstructorHelpers::FClassFinder<ADroneProjectile> ProjectileClassFound(TEXT("/Game/TopDownCPP/Blueprints/Projectiles/Base"));

	if (ProjectileClassFound.Succeeded()) {
		projectileClass = ProjectileClassFound.Class;
	}

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
	MoveSpeed = 800.0f;
	bCanFire = true;
	GunOffset = FVector(100.f, 0.f, 0.f);
	FireRate = 0.5;
}

ADroneRPGCharacter* ADroneRPGPlayerController::GetDrone() {
	return Cast<ADroneRPGCharacter>(GetCharacter());
}

void ADroneRPGPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (GetCharacter() != NULL) {
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

		GetCharacter()->SetActorRotation(FMath::Lerp(mActorRotation, lookAt, 0.10f));

		// Try and fire a shot
		FireShot(mActorRotation.Vector());
	}
}

void ADroneRPGPlayerController::FireShot(FVector FireDirection)
{
	// If it's ok to fire again
	if (isFiring && bCanFire)
	{
		// If we are pressing fire stick in a direction
		if (FireDirection.SizeSquared() > 0.0f)
		{
			const FRotator FireRotation = FireDirection.Rotation();

			// Spawn projectile at an offset from this pawn
			const FVector gunLocation = mActorLocation + FireRotation.RotateVector(GunOffset);

			UWorld* const World = GetWorld();
			if (World != NULL)
			{
				// spawn the projectile
				ADroneProjectile* projectile = World->SpawnActor<ADroneProjectile>(projectileClass, gunLocation, FireRotation);

				if (projectile != NULL) {
					projectile->SetShooter(GetCharacter());
					World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &ADroneRPGPlayerController::ShotTimerExpired, FireRate);

					// try and play the sound if specified
					if (FireSound != nullptr)
					{
						UGameplayStatics::PlaySoundAtLocation(this, FireSound, mActorLocation);
					}
					bCanFire = false;
				}
			}
		}
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

void ADroneRPGPlayerController::ShotTimerExpired()
{
	bCanFire = true;
}

void ADroneRPGPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	InputComponent->BindAction("UseTool", IE_Pressed, this, &ADroneRPGPlayerController::UseTool);
	InputComponent->BindAction("UseTool", IE_Released, this, &ADroneRPGPlayerController::StopUsingTool);

	// set up game play key bindings
	InputComponent->BindAxis(MoveForwardBinding);
	InputComponent->BindAxis(MoveRightBinding);
	InputComponent->BindAxis(FireForwardBinding);
	InputComponent->BindAxis(FireRightBinding);
}



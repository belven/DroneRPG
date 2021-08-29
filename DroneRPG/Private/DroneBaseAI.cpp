// Fill out your copyright notice in the Description page of Project Settings.


#include "DroneBaseAI.h"
#include <EngineUtils.h>
#include "../DroneRPGCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../DroneProjectile.h"
#include "Kismet/GameplayStatics.h"
#include <Kismet/KismetMathLibrary.h>

#define mActorLocation GetCharacter()->GetActorLocation()
#define mActorRotation GetCharacter()->GetActorRotation()
#define mDist FVector::Dist(targetObjective->GetActorLocation(), GetCharacter()->GetActorLocation())
#define  mObjectiveLocation targetObjective->GetActorLocation()

ADroneBaseAI::ADroneBaseAI() : Super()
{
	isMovingToObjective = false;
	minDistance = 800;

	static ConstructorHelpers::FClassFinder<ADroneProjectile> ProjectileClassFound(TEXT("/Game/TopDownCPP/Blueprints/Projectiles/Base"));

	if (ProjectileClassFound.Succeeded()) {
		projectileClass = ProjectileClassFound.Class;
	}

	bCanFire = true;
	GunOffset = FVector(100.f, 0.f, 0.f);
	FireRate = 0.5;
}

void ADroneBaseAI::CalculateObjective()
{
	//switch (GetCurrentGameMode()) {
	//case EGameModeType::AttackDefend:
	//case EGameModeType::Payload:
	//case EGameModeType::TeamDeathMatch:
	//case EGameModeType::Domination:
	//case EGameModeType::Hardpoint:
	//default:
	//};
	for (TActorIterator<AActor> actorItr(GetWorld()); actorItr; ++actorItr)
	{
		// Follow iterator object to my actual actor pointer
		AActor* actor = *actorItr;

		if (actor != GetCharacter() && actor->IsA(ADroneRPGCharacter::StaticClass())) {
			targetObjective = actor;
			break;
		}
	}

	if (targetObjective != NULL)
		currentState = EActionState::MovingToObjective;
}

void ADroneBaseAI::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	// Update Lookat on tick, so it's always facing correctly TODO make it follow the movement direction, if no target etc
	if (targetObjective != NULL) {
		lookAt = UKismetMathLibrary::FindLookAtRotation(mActorLocation, mObjectiveLocation);
		lookAt.Pitch = mActorRotation.Pitch;
		lookAt.Roll = mActorRotation.Roll;

		GetCharacter()->SetActorRotation(FMath::Lerp(mActorRotation, lookAt, 0.10f));
	}

	switch (currentState) {
	case EActionState::SearchingForObjective:
		CalculateObjective();
		break;
	case EActionState::MovingToObjective: 
		MoveToTarget();
		break;	
	case EActionState::AttackingTarget:
		AttackTarget();
		break;	
	}
}

void ADroneBaseAI::ShotTimerExpired()
{
	bCanFire = true;
}

void ADroneBaseAI::FireShot(FVector FireDirection)
{
	// If it's ok to fire again
	if (bCanFire)
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
					World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &ADroneBaseAI::ShotTimerExpired, FireRate);

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

void ADroneBaseAI::AttackTarget()
{
	FHitResult hit;
	FCollisionQueryParams params;
	params.AddIgnoredActor(GetCharacter());

	GetWorld()->LineTraceSingleByChannel(hit, mActorLocation, mObjectiveLocation, ECC_Pawn, params);

	if (hit.GetActor() == targetObjective)
	{
		FireShot(lookAt.Vector());
	}
	else {
		isMovingToObjective = false;
		currentState = EActionState::MovingToObjective;
	}

}

void ADroneBaseAI::MoveToTarget()
{
	if (mDist <= minDistance)
	{
		isMovingToObjective = false;
		if (targetObjective->IsA(ADroneRPGCharacter::StaticClass())) {
			currentState = EActionState::AttackingTarget;
		}
		else {
			currentState = EActionState::CapturingObjective;
		}
	}
	else if (!isMovingToObjective) {
		MoveToActor(targetObjective);
		isMovingToObjective = true;
	}

}
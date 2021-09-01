// Fill out your copyright notice in the Description page of Project Settings.


#include "DroneBaseAI.h"
#include <EngineUtils.h>
#include "../DroneRPGCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../DroneProjectile.h"
#include "Kismet/GameplayStatics.h"
#include <Kismet/KismetMathLibrary.h>
#include <Kismet/KismetArrayLibrary.h>
#include <Objective.h>

#define mDroneLocation GetCharacter()->GetActorLocation()
#define mDroneRotation GetCharacter()->GetActorRotation()
#define mDist(a, b) FVector::Dist(a, b)
#define  mObjectiveLocation targetObjective->GetActorLocation()
#define  mIsA(aObject, aClass)  aObject->IsA(aClass::StaticClass())

ADroneBaseAI::ADroneBaseAI() : Super()
{
	isMovingToObjective = false;
	minDistance = 1000;

	static ConstructorHelpers::FClassFinder<ADroneProjectile> ProjectileClassFound(TEXT("/Game/TopDownCPP/Blueprints/Projectiles/Base"));

	if (ProjectileClassFound.Succeeded()) {
		projectileClass = ProjectileClassFound.Class;
	}

	bCanFire = true;
	GunOffset = FVector(100.f, 0.f, 0.f);
	FireRate = 0.5;
	currentGameMode = EGameModeType::Domination;
}

void ADroneBaseAI::CalculateObjective()
{
	switch (GetCurrentGameMode()) {
	case EGameModeType::TeamDeathMatch:
		FindTarget();
		break;
	case EGameModeType::Domination:
		FindObjective();
		break;
	case EGameModeType::AttackDefend:
		FindObjective();
		break;
	case EGameModeType::Payload:
		FindObjective();
		break;
	case EGameModeType::Hardpoint:
		FindObjective();
		break;
	default:
		break;
	};

	if (targetObjective != NULL)
		currentState = EActionState::MovingToObjective;
}

void ADroneBaseAI::FindObjective() {
	TArray<AActor*> actors;

	for (TActorIterator<AActor> actorItr(GetWorld()); actorItr; ++actorItr)
	{
		actors.Add(*actorItr);
	}

	// TODO figure out random array sorting
	//const TArray<AActor*>& actorsSorted = actors;
	//UKismetArrayLibrary::Array_Shuffle(actorsSorted);

	for (AActor* actor : actors)
	{
		if (actor != GetCharacter() && mIsA(actor, AObjective)) {
			AObjective* objective = Cast<AObjective>(actor);

			targetObjective = actor;
			break;
		}
	}
}

void ADroneBaseAI::DroneAttacked(AActor* attacker) {
	if (target == NULL || (target != NULL && target != attacker)) {
		target = attacker;
	}
}

AActor* ADroneBaseAI::FindEnemyTarget(float distance) {
	TArray<AActor*> actors;

	for (TActorIterator<AActor> actorItr(GetWorld()); actorItr; ++actorItr)
	{
		actors.Add(*actorItr);
	}

	// TODO figure out random array sorting
	//const TArray<AActor*>& actorsSorted = actors;
	//UKismetArrayLibrary::Array_Shuffle(actorsSorted);

	for (AActor* actor : actors)
	{
		if (actor != GetCharacter() && mIsA(actor, ADroneRPGCharacter) &&
			(distance == 0 || mDist(mDroneLocation, actor->GetActorLocation()) <= distance)) {
			return actor;
		}
	}

	return NULL;
}

void ADroneBaseAI::FindTarget() {
	targetObjective = FindEnemyTarget();
}

void ADroneBaseAI::RotateToFace() {
	FVector targetLocation;
	FRotator targetRotation;

	if (currentState == EActionState::AttackingTarget) {
		if (targetObjective != NULL)
			targetLocation = mObjectiveLocation;
	}
	else if (IsTargetValid()) {
		targetLocation = target->GetTargetLocation();
	}
	else {
		targetLocation = GetCharacter()->GetActorForwardVector();
	}

	if (!targetLocation.IsNearlyZero()) {
		lookAt = UKismetMathLibrary::FindLookAtRotation(mDroneLocation, targetLocation);
		lookAt.Pitch = mDroneRotation.Pitch;
		lookAt.Roll = mDroneRotation.Roll;
		targetRotation = lookAt;
	}
	else {
		targetRotation = GetCharacter()->GetActorForwardVector().Rotation();
		targetRotation.Pitch = mDroneRotation.Pitch;
		targetRotation.Roll = mDroneRotation.Roll;
	}

	GetCharacter()->SetActorRotation(FMath::Lerp(mDroneRotation, targetRotation, 0.10f));
}

void ADroneBaseAI::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Update Lookat on tick, so it's always facing correctly TODO make it follow the movement direction, if no target etc
	RotateToFace();

	switch (currentState) {
	case EActionState::SearchingForObjective:
		CalculateObjective();
		break;
	case EActionState::MovingToObjective:
		MoveToObjective();
		break;
	case EActionState::AttackingTarget:
		AttackingTarget();
		break;
	case EActionState::CapturingObjective:
		CapturingObjective();
		break;
	case EActionState::EvadingDamage:
		break;
	case EActionState::ReturingToBase:
		break;
	default:
		break;
	}
}

void ADroneBaseAI::AttackingTarget() {
	if (targetObjective->IsActorBeingDestroyed()) {
		currentState = EActionState::SearchingForObjective;
		targetObjective = NULL;
	}
	else {
		AttackTarget(targetObjective);
	}
}

bool ADroneBaseAI::IsTargetValid() {
	return target != NULL && !target->IsActorBeingDestroyed();
}

void ADroneBaseAI::CapturingObjective() {
	AObjective* currentObjective = Cast<AObjective>(targetObjective);

	if (mDist(mDroneLocation, mObjectiveLocation) >= minDistance) {
		MoveToActor(targetObjective);
		target = NULL;
	}
	else if (currentObjective != NULL && currentObjective->HasCompleteControl(team)) {
		currentState = EActionState::SearchingForObjective;
	}
	else if (IsTargetValid()) {
		if (!AttackTarget(target, false)) {
			target = NULL;
		}
	}
	else {
		AActor* targetFound = FindEnemyTarget(minDistance);

		if (targetFound != NULL)
			target = targetFound;
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
			const FVector gunLocation = mDroneLocation + FireRotation.RotateVector(GunOffset);

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
						UGameplayStatics::PlaySoundAtLocation(this, FireSound, mDroneLocation);
					}
					bCanFire = false;
				}
			}
		}
	}
}

bool ADroneBaseAI::AttackTarget(AActor* targetToAttack, bool moveIfCantSee)
{
	FHitResult hit;
	FCollisionQueryParams params;
	params.AddIgnoredActor(GetCharacter());

	GetWorld()->LineTraceSingleByChannel(hit, mDroneLocation, targetToAttack->GetActorLocation(), ECC_Pawn, params);

	if (hit.GetActor() == targetToAttack)
	{
		GetCharacter()->GetMovementComponent()->StopActiveMovement();
		FireShot(lookAt.Vector());
		return true;
	}
	else {
		if (moveIfCantSee)
			MoveToActor(targetToAttack);
		return false;
	}
	return false;
}

void ADroneBaseAI::MoveToObjective()
{
	if (mDist(mObjectiveLocation, mDroneLocation) <= 400)
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
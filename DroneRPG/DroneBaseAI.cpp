#include "DroneBaseAI.h"
#include <EngineUtils.h>
#include "DroneRPGCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DroneProjectile.h"
#include "Kismet/GameplayStatics.h"
#include <Kismet/KismetMathLibrary.h>
#include <Kismet/KismetArrayLibrary.h>
#include "Objective.h"
#include "FunctionLibrary.h"

#define  mObjectiveLocation targetObjective->GetActorLocation()

ADroneBaseAI::ADroneBaseAI() : Super()
{
	minCaptureDistance = 400;
	targetRange = 3000;

	static ConstructorHelpers::FClassFinder<ADroneProjectile> ProjectileClassFound(TEXT("/Game/TopDownCPP/Blueprints/Projectiles/Base"));

	if (ProjectileClassFound.Succeeded()) {
		projectileClass = ProjectileClassFound.Class;
	}

	bCanFire = true;
	canCheckForEnemies = true;
	GunOffset = FVector(100.f, 0.f, 0.f);
	FireRate = 0.5;
	currentGameMode = EGameModeType::Domination;
}

void ADroneBaseAI::CalculateObjective()
{
	// Set objective to NULL, as it may still be set from a previous Calculate Objective
	targetObjective = NULL;

	// Calculate the game mode and decide what actions to perform
	switch (GetCurrentGameMode()) {
	case EGameModeType::TeamDeathMatch:
		// We only need to find enemies in the team death match
		FindTarget();
		break;
	case EGameModeType::Domination:
	case EGameModeType::AttackDefend:
	case EGameModeType::Payload:
	case EGameModeType::Hardpoint:
		// In all other cases we will need to find an objective TODO:, this may need to change in game modes such as AttackDefend
		FindObjective();
		break;
	default:
		mAddOnScreenDebugMessage("I have no idea what I'm supposed to be doing!");
		break;
	};
}

void ADroneBaseAI::FindObjective() {
	TArray<AObjective*> objectives = mGetActorsInWorld<AObjective>(GetWorld());

	mShuffleArray<AObjective*>(objectives);

	for (AObjective* objective : objectives)
	{
		// Check if we don't already control the objective, if we do, then we'll be picking one to defend later
		if (!objective->HasCompleteControl(GetDrone()->GetTeam())) {
			targetObjective = objective;
			break;
		}
	}

	// If the objective is null and we found any objectives, then find one to defend
	if (targetObjective == NULL && objectives.Num() > 0) {

		targetObjective = UFunctionLibrary::GetRandomObject<AObjective*>(objectives);
		MoveToActor(targetObjective);
		currentState = EActionState::DefendingObjective;
	}
	// If we have an objective, then it wasn't claimed by this team, so head towards it
	else if (targetObjective != NULL) {
		MoveToActor(targetObjective);
		currentState = EActionState::CapturingObjective;
	}
}

void ADroneBaseAI::DroneAttacked(AActor* attacker) {
	// If we don't have a target OR we have one but it's different to the one we have, then target it
	ADroneRPGCharacter* droneTarget = Cast<ADroneRPGCharacter>(target);
	ADroneRPGCharacter* droneAttacker = Cast<ADroneRPGCharacter>(attacker);

	// We have no target
	if (droneTarget == NULL) {
		target = droneAttacker;
	}
	else if (droneTarget != droneAttacker) {
		FHitResult hit = LinetraceToLocation(droneTarget->GetActorLocation());

		// Something else has attacked us and our current target is out of line of sight or dead
		if (!droneTarget->IsAlive() || hit.GetActor() != droneTarget) {
			target = droneAttacker;
		}
	}
}

void ADroneBaseAI::BeginPlay() {
	Super::BeginPlay();

	// We need to bind to the Objective Claimed method on all the objectives, so we can track when they get taken
	// This allows the bot to move an objective as an enemy tries to take it
	for (AObjective* objective : mGetActorsInWorld<AObjective>(GetWorld()))
	{
		// Bind to the Objective Claimed method on the objective
		objective->OnObjectiveClaimed.AddDynamic(this, &ADroneBaseAI::ObjectiveTaken);
	}
}

void ADroneBaseAI::ObjectiveTaken(AObjective* objective) {

	// Check if the objective isn't owned by us, if it is we don't care!
	if (!objective->HasCompleteControl(GetDrone()->GetTeam()) && currentState != EActionState::CapturingObjective) {
		currentState = EActionState::CapturingObjective;
		targetObjective = objective;
	}
}

AActor* ADroneBaseAI::FindEnemyTarget(float distance) {
	TArray<ADroneRPGCharacter*> drones = GetDrone()->GetDronesInArea();//mGetActorsInWorld<ADroneRPGCharacter>(GetWorld());

	mShuffleArray<ADroneRPGCharacter*>(drones);

	for (ADroneRPGCharacter* drone : drones)
	{
		// Check if the actor isn't us and is a drone
		// Check if the distance is 0 OR we're within the given range
		if (drone != GetCharacter() &&
			(distance == 0 || mDist(mDroneLocation, drone->GetActorLocation()) <= distance)) {

			// Check if the drone found is an enemy
			if (drone->GetTeam() != GetDrone()->GetTeam())
				return drone;
		}
	}

	// In all other cases, return NULL
	return NULL;
}

void ADroneBaseAI::FindTarget() {
	targetObjective = FindEnemyTarget();
	target = Cast<ADroneRPGCharacter>(targetObjective);
	currentState = EActionState::AttackingTarget;
}

void ADroneBaseAI::RotateToFace() {
	FVector targetLocation;
	FRotator targetRotation;

	// If we have a target, turn to face it
	if (IsTargetValid()) {
		targetLocation = target->GetTargetLocation();
	}
	// Otherwise, if we have an objective to head to, face that
	else if (targetObjective != NULL) {
		targetLocation = mObjectiveLocation;
	}
	// All other cases, which there shouldn't be, eyes front!
	else {
		targetLocation = GetCharacter()->GetActorForwardVector();
	}

	// If we set a location, then calculate a loot at rotation
	if (!targetLocation.IsNearlyZero()) {
		// Calculate the angle to look at our target
		lookAt = UKismetMathLibrary::FindLookAtRotation(mDroneLocation, targetLocation);
		lookAt.Pitch = mDroneRotation.Pitch;
		lookAt.Roll = mDroneRotation.Roll;
		targetRotation = lookAt;
	}
	else {
		// Set the angle to be forward facing TODO:, this doesn't work well and needs to face movement direction!
		targetRotation = GetCharacter()->GetActorForwardVector().Rotation();
		targetRotation.Pitch = mDroneRotation.Pitch;
		targetRotation.Roll = mDroneRotation.Roll;
	}

	// Update the rotation
	//GetCharacter()->SetActorRotation(FMath::Lerp(mDroneRotation, targetRotation, 0.50f));
	GetCharacter()->SetActorRotation(targetRotation);
}

void ADroneBaseAI::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// We don't do anything if we're dead!
	if (GetDrone()->IsAlive()) {
		if (!IsTargetValid()) {
			target = NULL;
		}

		// Update Lookat on tick, so it's always facing correctly TODO: make it follow the movement direction, if no target etc
		RotateToFace();

		PerformActions();
	}
	else if(currentState != EActionState::SearchingForObjective)
	{
		// If this is hit, then we've likely died and need to reset our state!
		currentState = EActionState::SearchingForObjective;
	}
}

void ADroneBaseAI::PerformActions() {
	// Do state machine things!
	switch (currentState) {
	case EActionState::SearchingForObjective:
		CalculateObjective();
		break;
	case EActionState::AttackingTarget:
		AttackingTarget();
		break;
	case EActionState::CapturingObjective:
		CapturingObjective();
		break;
	case EActionState::DefendingObjective:
		DefendingObjective();
		break;
	case EActionState::EvadingDamage:
		EvadingDamage();
		break;
	case EActionState::ReturingToBase:
		ReturningToBase();
		break;
	default:
		// Something went wrong!!
		mAddOnScreenDebugMessage("I have no idea what I'm supposed to be doing!");
		break;
	}
}

void ADroneBaseAI::DefendingObjective() {
	AObjective* currentObjective = Cast<AObjective>(targetObjective);

	// Have we gone too far from our objective? If so move closer
	if (mDist(mDroneLocation, mObjectiveLocation) >= minCaptureDistance) {
		MoveToActor(targetObjective);
	}
	// Are we being attacked
	else if (IsTargetValid()) {
		if (!AttackTarget(target, false)) {

		}
	}
	//Is there an enemy nearby the area
	else if (canCheckForEnemies) {
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_CanCheckForEnemies, this, &ADroneBaseAI::CanCheckForEnemies, 0.5f);
		canCheckForEnemies = false;
		AActor* targetFound = FindEnemyTarget(targetRange);

		if (targetFound != NULL)
			target = targetFound;
	}
}

void ADroneBaseAI::ReturningToBase() {

}

void ADroneBaseAI::EvadingDamage() {
	//if(GetDrone()->GetCurrentStats() < ) etc
}

void ADroneBaseAI::AttackingTarget() {
	// Check the target is still valid
	if (!IsTargetValid()) {
		currentState = EActionState::SearchingForObjective;
		targetObjective = NULL;
	}
	// Shoot the current target
	else {
		AttackTarget(targetObjective);
	}
}

FHitResult ADroneBaseAI::LinetraceToLocation(FVector location) {
	FHitResult hit;
	FCollisionQueryParams params;
	params.AddIgnoredActor(GetCharacter());

	GetWorld()->LineTraceSingleByChannel(hit, mDroneLocation, location, ECC_Pawn, params);

	return hit;
}

bool ADroneBaseAI::AttackTarget(AActor* targetToAttack, bool moveIfCantSee)
{
	FHitResult hit = LinetraceToLocation(targetToAttack->GetActorLocation());

	// Do we have line of sight to our target?
	if (hit.GetActor() == targetToAttack)
	{
		//GetCharacter()->GetMovementComponent()->StopActiveMovement();
		FireShot(lookAt.Vector());
		return true;
	}
	// We don't have line of sight
	else {
		// Only move to the target if told to do so
		if (moveIfCantSee)
			MoveToActor(targetToAttack);
		return false;
	}
	return false;
}

bool ADroneBaseAI::IsTargetValid() {
	ADroneRPGCharacter* droneTarget = Cast<ADroneRPGCharacter>(target);

	if (droneTarget != NULL && !droneTarget->IsActorBeingDestroyed()) {
		return droneTarget->IsAlive();
	}

	return false;
}

void ADroneBaseAI::CapturingObjective() {
	AObjective* currentObjective = Cast<AObjective>(targetObjective);

	// Have we gone too far from our objective? If so move closer
	if (mDist(mDroneLocation, mObjectiveLocation) >= minCaptureDistance) {
		MoveToActor(targetObjective);
	}
	// Have we claimed the current objective?
	else if (currentObjective != NULL && currentObjective->HasCompleteControl(GetDrone()->GetTeam())) {
		currentState = EActionState::SearchingForObjective;
	}

	// Do we have a valid target
	if (IsTargetValid()) {

		// Attempt to attack the target, don't move to the target if we can't see it
		// TODO make it so we move to the target, as long as we're in range of the objective still
		if (!AttackTarget(target, false)) {
			// We're in here due to us loosing line of sight to the target
			target = NULL;
		}
	}
	//Is there an enemy nearby the area
	else if (canCheckForEnemies) {
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_CanCheckForEnemies, this, &ADroneBaseAI::CanCheckForEnemies, 1.0f);
		canCheckForEnemies = false;
		AActor* targetFound = FindEnemyTarget(targetRange);

		if (targetFound != NULL) {
			FHitResult hit = LinetraceToLocation(targetFound->GetActorLocation());

			if (hit.GetActor() == targetFound)
				target = targetFound;
		}
	}
}

void ADroneBaseAI::ShotTimerExpired()
{
	bCanFire = true;
}

void ADroneBaseAI::CanCheckForEnemies()
{
	canCheckForEnemies = true;
}

ADroneRPGCharacter* ADroneBaseAI::GetDrone()
{
	return Cast<ADroneRPGCharacter>(GetCharacter());
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

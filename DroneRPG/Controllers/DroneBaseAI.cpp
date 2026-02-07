#pragma once
#include "DroneBaseAI.h"
#include <Kismet/KismetMathLibrary.h>

#include "NavigationSystem.h"
#include "DroneRPG/DroneRPGCharacter.h"
#include "DroneRPG/Utilities/Enums.h"
#include "DroneRPG/Utilities/FunctionLibrary.h"
#include "DroneRPG/LevelActors/Objective.h"
#include "DroneRPG/LevelActors/RespawnPoint.h"
#include "DroneRPG/Weapons/DroneProjectile.h"
#include "DroneRPG/Weapons/Weapon.h"
#include "GameFramework/Character.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

#define  mObjectiveLocation targetObjective->GetActorLocation()

ADroneBaseAI::ADroneBaseAI(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), lookAt(), lastLocation(), isFiring(false), targetObjective(nullptr), target(nullptr), currentState(), previousState()
{
	PrimaryActorTick.TickInterval = 0.1;
	minCaptureDistance = 650;
	targetRange = 3000;

	canCheckForEnemies = true;
	canPerformActions = true;
	currentGameMode = EGameModeType::Domination;
}

void ADroneBaseAI::CheckLastLocation() {
	if (lastLocation == mDroneLocation) {
		GetDrone()->Respawn();
	}
	lastLocation = FVector::ZeroVector;
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
		// In all other cases we will need to find an objective TODO: this may need to change in game modes such as AttackDefend
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
		MoveToObjective();
		currentState = EActionState::DefendingObjective;
	}
	// If we have an objective, then it wasn't claimed by this team, so head towards it
	else if (targetObjective != NULL) {
		MoveToObjective();
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
		// Something else has attacked us and our current target is out of line of sight or dead
		if (!droneTarget->IsAlive() || !CanSee(droneTarget, mDroneLocation)) {
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
	bool hasControl = objective->HasCompleteControl(GetDrone()->GetTeam());

	// Check if the objective isn't owned by us, if it is we don't care!
	if (!hasControl && currentState != EActionState::CapturingObjective) {
		currentState = EActionState::CapturingObjective;
		targetObjective = objective;
	}
	else if (hasControl && currentState == EActionState::CapturingObjective && targetObjective == objective) {
		currentState = EActionState::SearchingForObjective;
	}
}

AActor* ADroneBaseAI::FindEnemyTarget(float distance) {
	TArray<ADroneRPGCharacter*> drones = mGetEnemiesInRadius(distance, mDroneLocation, GetDrone()->GetTeam());

	if (drones.Num() > 0) {
		mShuffleArray<ADroneRPGCharacter*>(drones);
		return UFunctionLibrary::GetRandomObject<ADroneRPGCharacter*>(drones);
	}

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
		targetLocation = GetPredictedLocation(target);
	}
	// Otherwise, if we have an objective to head to, face that
	else if (targetObjective != NULL) {
		targetLocation = mObjectiveLocation;
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
		targetRotation = GetDrone()->GetVelocity().Rotation();
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
		if (target != NULL && !IsTargetValid()) {
			target = NULL;
		}

		if (lastLocation.IsZero()) {
			lastLocation = mDroneLocation;
			mSetTimer(TimerHandle_CheckLastLocation, &ADroneBaseAI::CheckLastLocation, 15.0f);
		}

		RotateToFace();

		if (currentState != EActionState::ReturningToBase && !GetDrone()->IsHealthy()) {
			currentState = EActionState::ReturningToBase;
		}
		else if (currentState != EActionState::EvadingDamage && currentState != EActionState::ReturningToBase && IsTargetValid()) {
			currentState = EActionState::EvadingDamage;
			GetDrone()->GetMovementComponent()->StopActiveMovement();
		}

		if (canPerformActions) {
			PerformActions();
		}
	}
	else if (currentState != EActionState::SearchingForObjective)
	{
		// If this is hit, then we've likely died and need to reset our state!
		currentState = EActionState::SearchingForObjective;
	}
}

void ADroneBaseAI::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	//GetDrone()->GetCameraBoom()->TargetArmLength = GetDrone()->GetWeapon()->GetRange() * 1.3;
	GetDrone()->GetCameraBoom()->TargetArmLength = 10000;
}

void ADroneBaseAI::PerformActions() {
	canPerformActions = false;
	mSetTimer(TimerHandle_CanPerformActions, &ADroneBaseAI::CanPerformActions, FMath::RandRange(0.1f, 0.15f));

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
	case EActionState::ReturningToBase:
		ReturningToBase();
		break;
	default:
		// Something went wrong!!
		mAddOnScreenDebugMessage("I have no idea what I'm supposed to be doing!");
		break;
	}
}

void ADroneBaseAI::MoveToObjective() {
	float distance;

	AKeyActor* keyActor = Cast<AKeyActor>(targetObjective);

	if (keyActor != NULL) {
		distance = keyActor->GetSize() * 0.7f;
	}
	else {
		distance = 600;
	}

	FNavLocation loc;
	mRandomReachablePointInRadius(targetObjective->GetActorLocation(), distance, loc);
	MoveToLocation(loc);
}

void ADroneBaseAI::DefendingObjective() {
	//AObjective* currentObjective = Cast<AObjective>(targetObjective);

	// Have we gone too far from our objective? If so move closer
	if (mDist(mDroneLocation, mObjectiveLocation) >= minCaptureDistance) {
		MoveToObjective();
	}

	// Do we have a valid target
	// Is there an enemy nearby the area
	if (!ShootAttacker() && canCheckForEnemies) {
		GetEnemiesInArea();
	}
}

void ADroneBaseAI::ReturningToBase() {
	if (!GetDrone()->IsHealthy()) {
		FVector loc = GetDrone()->GetRespawnPoint()->GetActorLocation();
		MoveToLocation(loc);
	}
	else {
		currentState = EActionState::SearchingForObjective;
	}

	// Do we have a valid target
	// Is there an enemy nearby the area
	if (!ShootAttacker() && canCheckForEnemies) {
		GetEnemiesInArea();
	}
}

bool ADroneBaseAI::ShootAttacker() {
	if (IsTargetValid()) {
		// Attempt to attack the target, don't move to the target if we can't see it
		if (!AttackTarget(target, false)) {
			// We're in here due to us loosing line of sight to the target
			target = NULL;
		}
		else {
			return true;
		}
	}
	return false;
}

void ADroneBaseAI::EvadingDamage() {
	if (IsTargetValid()) {
		if (GetDrone()->GetVelocity().IsNearlyZero()) {
			int32 closeDistance = GetDrone()->GetWeapon()->GetRange() * 0.8;
			int32 count = 0;
			FNavLocation loc;
			mRandomReachablePointInRadius(GetDrone()->GetActorLocation(), closeDistance, loc);

			while (mDist(target->GetActorLocation(), loc) <= closeDistance / 2 && !CanSee(target, loc) && count < 20) {
				mRandomReachablePointInRadius(GetDrone()->GetActorLocation(), closeDistance, loc);
				count++;
			}
			MoveToLocation(loc);
		}

		ShootAttacker();
	}
	else {
		currentState = EActionState::SearchingForObjective;
	}
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

FHitResult ADroneBaseAI::LineTraceToLocation(const FVector& startLoc, const FVector& endLocation) {
	FHitResult hit;
	FCollisionQueryParams params;
	params.AddIgnoredActor(GetCharacter());

	GetWorld()->LineTraceSingleByChannel(hit, startLoc, endLocation, ECC_Pawn, params);

	return hit;
}

bool ADroneBaseAI::CanSee(AActor* other, const FVector& startLoc) {
	FHitResult hit = LineTraceToLocation(startLoc, other->GetActorLocation());

	// Do we have line of sight to our target?
	if (hit.GetActor() == other)
	{
		return true;
	}
	return false;
}

FVector ADroneBaseAI::GetPredictedLocation(AActor* actor) {
	float time = mDist(mDroneLocation, actor->GetActorLocation()) / ADroneProjectile::Default_Initial_Speed;
	//time = FMath::RandRange(time * 0.9f, time * 1.1f);
	return actor->GetActorLocation() + (actor->GetVelocity() * time);
}

bool ADroneBaseAI::AttackTarget(AActor* targetToAttack, bool moveIfCantSee)
{
	// Do we have line of sight to our target?
	if (CanSee(targetToAttack, mDroneLocation))
	{
		FireShot(lookAt.Vector());
		return true;
	}
	// We don't have line of sight

		// Only move to the target if told to do so
	if (moveIfCantSee) {
		MoveToActor(targetToAttack);
	}
	return false;
}

bool ADroneBaseAI::IsTargetValid() {
	ADroneRPGCharacter* droneTarget = Cast<ADroneRPGCharacter>(target);

	if (droneTarget != NULL && !droneTarget->IsActorBeingDestroyed()
		&& mDist(droneTarget->GetActorLocation(), mDroneLocation) <= GetDrone()->GetWeapon()->GetRange()) {
		return droneTarget->IsAlive();
	}

	return false;
}

void ADroneBaseAI::CapturingObjective() {
	AObjective* currentObjective = Cast<AObjective>(targetObjective);

	// Have we gone too far from our objective? If so move closer
	if (mDist(mDroneLocation, mObjectiveLocation) >= minCaptureDistance) {
		MoveToObjective();
	}
	// Have we claimed the current objective?
	else if (currentObjective != NULL && currentObjective->HasCompleteControl(GetDrone()->GetTeam())) {
		currentState = EActionState::SearchingForObjective;
	}

	// Do we have a valid target
	// Is there an enemy nearby the area
	if (!ShootAttacker() && canCheckForEnemies) {
		GetEnemiesInArea();
	}
}

bool ADroneBaseAI::GetEnemiesInArea() {
	mSetTimer(TimerHandle_CanCheckForEnemies, &ADroneBaseAI::CanCheckForEnemies, 0.1f);
	canCheckForEnemies = false;
	AActor* targetFound = FindEnemyTarget(GetDrone()->GetWeapon()->GetRange());

	if (targetFound != NULL) {
		if (CanSee(targetFound, mDroneLocation)) {
			target = targetFound;
			return true;
		}
	}

	return false;
}

void ADroneBaseAI::CanCheckForEnemies()
{
	canCheckForEnemies = true;
}

void ADroneBaseAI::CanPerformActions()
{
	canPerformActions = true;
}

ADroneRPGCharacter* ADroneBaseAI::GetDrone()
{
	return Cast<ADroneRPGCharacter>(GetCharacter());
}

void ADroneBaseAI::FireShot(const FVector& FireDirection)
{
	GetDrone()->GetWeapon()->FireShot(FireDirection, target);
}
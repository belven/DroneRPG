// Fill out your copyright notice in the Description page of Project Settings.


#include "DroneBaseAI.h"
#include <EngineUtils.h>
#include "DroneRPGCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DroneProjectile.h"
#include "Kismet/GameplayStatics.h"
#include <Kismet/KismetMathLibrary.h>
#include <Kismet/KismetArrayLibrary.h>
#include "Objective.h"

#define mDroneLocation GetCharacter()->GetActorLocation()
#define mDroneRotation GetCharacter()->GetActorRotation()
#define mDist(a, b) FVector::Dist(a, b)
#define  mObjectiveLocation targetObjective->GetActorLocation()
#define  mIsA(aObject, aClass)  aObject->IsA(aClass::StaticClass())
#define  mAddOnScreenDebugMessage(text) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT(text)));

ADroneBaseAI::ADroneBaseAI() : Super()
{
	minCaptureDistance = 400;
	targetRange = 1500;

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
		// In all other cases we will need to find an objective TODO, this may need to change in game modes such as AttackDefend
		FindObjective();
		break;
	default:
		mAddOnScreenDebugMessage("I have no idea what I'm supposed to be doing!");
		break;
	};
}

TArray<AActor*> ADroneBaseAI::GetActorsInWorld() {
	TArray<AActor*> actors;

	for (TActorIterator<AActor> actorItr(GetWorld()); actorItr; ++actorItr)
	{
		actors.Add(*actorItr);
	}

	return actors;
}

void ADroneBaseAI::FindObjective() {
	TArray<AActor*> actors = GetActorsInWorld();
	TArray<AObjective*> objectives;

	// TODO figure out random array sorting
	//const TArray<AActor*>& actorsSorted = actors;
	//UKismetArrayLibrary::Array_Shuffle(actorsSorted);

	for (AActor* actor : actors)
	{
		// Check we're not finding ourselves and the actor is an objective
		if (actor != GetCharacter() && mIsA(actor, AObjective)) {
			AObjective* objective = Cast<AObjective>(actor);
			objectives.Add(objective);

			// Check if we don't already control the objective, if we do, then we'll be picking one to defend later
			if (!objective->HasCompleteControl(GetDrone()->GetTeam())) {
				targetObjective = actor;
				break;
			}
		}
	}

	// If the objective is null and we found any objectives, then find one to defend
	if (targetObjective == NULL && objectives.Num() > 0) {

		// Pick an objective to head to, to defend TODO make this pick a random objective
		targetObjective = objectives[0];
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
	// TODO, do we need the second part here??
	if (target == NULL || (target != NULL && target != attacker)) {
		target = attacker;
	}
}

void ADroneBaseAI::BeginPlay() {
	Super::BeginPlay();

	// We need to bind to the Objective Claimed method on all the objectives, so we can track when they get taken
	// This allows the bot to move an objective as an enemy tries to take it
	for (AActor* actor : GetActorsInWorld())
	{
		// Check if the actor is an Objective
		if (mIsA(actor, AObjective)) {
			AObjective* objective = Cast<AObjective>(actor);

			// Bind to the Objective Claimed method on the objective
			objective->OnObjectiveClaimed.AddDynamic(this, &ADroneBaseAI::ObjectiveTaken);
		}
	}
}

void ADroneBaseAI::ObjectiveTaken(AObjective* objective) {

	// Check if the objective isn't owned by us, if it is we don't care!
	if (!objective->HasCompleteControl(GetDrone()->GetTeam())) {
		currentState = EActionState::CapturingObjective;
		targetObjective = objective;
	}
}

AActor* ADroneBaseAI::FindEnemyTarget(float distance) {
	TArray<AActor*> actors = GetActorsInWorld();

	// TODO figure out random array sorting
	//const TArray<AActor*>& actorsSorted = actors;
	//UKismetArrayLibrary::Array_Shuffle(actorsSorted);

	for (AActor* actor : actors)
	{
		// Check if the actor isn't us and is a drone
		// Check if the distance is 0 OR we're within the given range
		if (actor != GetCharacter() && mIsA(actor, ADroneRPGCharacter) &&
			(distance == 0 || mDist(mDroneLocation, actor->GetActorLocation()) <= distance)) {
			ADroneRPGCharacter* drone = Cast<ADroneRPGCharacter>(actor);

			// Check if the drone found is an enemy
			if (drone->GetTeam() != GetDrone()->GetTeam())
				return actor;
		}
	}

	// In all other cases, return NULL
	return NULL;
}

void ADroneBaseAI::FindTarget() {
	targetObjective = FindEnemyTarget();
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
		// Set the angle to be forward facing TODO, this doesn't work well and needs to face movement direction!
		targetRotation = GetCharacter()->GetActorForwardVector().Rotation();
		targetRotation.Pitch = mDroneRotation.Pitch;
		targetRotation.Roll = mDroneRotation.Roll;
	}

	// Update the rotation
	GetCharacter()->SetActorRotation(FMath::Lerp(mDroneRotation, targetRotation, 0.10f));
}

void ADroneBaseAI::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Update Lookat on tick, so it's always facing correctly TODO make it follow the movement direction, if no target etc
	RotateToFace();

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
		target = NULL;
	}
	// Are we being attacked
	else if (IsTargetValid()) {
		if (!AttackTarget(target, false)) {
			target = NULL;
		}
	}
	//Is there an enemy nearby the area
	else {
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
	// Check the target is still valid TODO need to check health etc. here
	if (targetObjective->IsActorBeingDestroyed()) {
		currentState = EActionState::SearchingForObjective;
		targetObjective = NULL;
	}
	// Shoot the current target
	else {
		AttackTarget(targetObjective);
	}
}

bool ADroneBaseAI::IsTargetValid() {
	return target != NULL && !target->IsActorBeingDestroyed();
}

void ADroneBaseAI::CapturingObjective() {
	AObjective* currentObjective = Cast<AObjective>(targetObjective);

	// Have we gone too far from our objective? If so move closer
	if (mDist(mDroneLocation, mObjectiveLocation) >= minCaptureDistance) {
		MoveToActor(targetObjective);
		target = NULL;
	}
	// Have we claimed the current objective?
	else if (currentObjective != NULL && currentObjective->HasCompleteControl(GetDrone()->GetTeam())) {
		currentState = EActionState::SearchingForObjective;
	}
	// Are we being attacked
	else if (IsTargetValid()) {
		if (!AttackTarget(target, false)) {
			target = NULL;
		}
	}
	//Is there an enemy nearby the area
	else {
		AActor* targetFound = FindEnemyTarget(targetRange);

		if (targetFound != NULL)
			target = targetFound;
	}
}

void ADroneBaseAI::ShotTimerExpired()
{
	bCanFire = true;
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

bool ADroneBaseAI::AttackTarget(AActor* targetToAttack, bool moveIfCantSee)
{
	FHitResult hit;
	FCollisionQueryParams params;
	params.AddIgnoredActor(GetCharacter());

	GetWorld()->LineTraceSingleByChannel(hit, mDroneLocation, targetToAttack->GetActorLocation(), ECC_Pawn, params);

	// Do we have line of sight to our target?
	if (hit.GetActor() == targetToAttack)
	{
		GetCharacter()->GetMovementComponent()->StopActiveMovement();
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
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
	targetObjective = NULL;

	switch (GetCurrentGameMode()) {
	case EGameModeType::TeamDeathMatch:
		FindTarget();
		break;
	case EGameModeType::Domination:
	case EGameModeType::AttackDefend:
	case EGameModeType::Payload:
	case EGameModeType::Hardpoint:
		FindObjective();
		break;
	default:
		mAddOnScreenDebugMessage("I have no idea what I'm supposed to be doing!");
		break;
	};

	//if (targetObjective != NULL)
		//currentState = EActionState::MovingToObjective;
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
		if (actor != GetCharacter() && mIsA(actor, AObjective)) {
			AObjective* objective = Cast<AObjective>(actor);
			objectives.Add(objective);

			if (!objective->HasCompleteControl(GetDrone()->GetTeam())) {
				targetObjective = actor;
				break;
			}
		}
	}

	if (targetObjective == NULL && objectives.Num() > 0) {
		targetObjective = objectives[0];
		MoveToActor(targetObjective);
		currentState = EActionState::DefendingObjective;
	}
	else if (targetObjective != NULL) {
		MoveToActor(targetObjective);
		currentState = EActionState::CapturingObjective;
	}
}

void ADroneBaseAI::DroneAttacked(AActor* attacker) {
	if (target == NULL || (target != NULL && target != attacker)) {
		target = attacker;
	}
}

void ADroneBaseAI::BeginPlay() {
	Super::BeginPlay();

	for (AActor* actor : GetActorsInWorld())
	{
		if (actor != GetCharacter() && mIsA(actor, AObjective)) {
			AObjective* objective = Cast<AObjective>(actor);
			objective->OnObjectiveClaimed.AddDynamic(this, &ADroneBaseAI::ObjectiveTaken);
		}
	}
}

void ADroneBaseAI::ObjectiveTaken(AObjective* objective) {
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
		if (actor != GetCharacter() && mIsA(actor, ADroneRPGCharacter) &&
			(distance == 0 || mDist(mDroneLocation, actor->GetActorLocation()) <= distance)) {
			ADroneRPGCharacter* drone = Cast<ADroneRPGCharacter>(actor);

			if (drone->GetTeam() != GetDrone()->GetTeam())
				return actor;
		}
	}

	return NULL;
}

void ADroneBaseAI::FindTarget() {
	targetObjective = FindEnemyTarget();
	currentState = EActionState::AttackingTarget;
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

	if (mDist(mDroneLocation, mObjectiveLocation) >= minCaptureDistance) {
		MoveToActor(targetObjective);
		target = NULL;
	}
	else if (IsTargetValid()) {
		if (!AttackTarget(target, false)) {
			target = NULL;
		}
	}
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

	if (mDist(mDroneLocation, mObjectiveLocation) >= minCaptureDistance) {
		MoveToActor(targetObjective);
		target = NULL;
	}
	else if (currentObjective != NULL && currentObjective->HasCompleteControl(GetDrone()->GetTeam())) {
		currentState = EActionState::SearchingForObjective;
	}
	else if (IsTargetValid()) {
		if (!AttackTarget(target, false)) {
			target = NULL;
		}
	}
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


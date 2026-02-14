#pragma once
#include "DroneBaseAI.h"
#include <Kismet/KismetMathLibrary.h>
#include "NavigationSystem.h"
#include "DroneRPG/DroneRPG.h"
#include "DroneRPG/DroneRPGCharacter.h"
#include "DroneRPG/Components/HealthComponent.h"
#include "DroneRPG/Utilities/Enums.h"
#include "DroneRPG/Utilities/FunctionLibrary.h"
#include "DroneRPG/LevelActors/Objective.h"
#include "DroneRPG/LevelActors/RespawnPoint.h"
#include "DroneRPG/Weapons/DroneProjectile.h"
#include "DroneRPG/Weapons/Weapon.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

#define  mObjectiveLocation targetObjective->GetActorLocation()

ADroneBaseAI::ADroneBaseAI(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), lookAt(), lastLocation(), isFiring(false), targetObjective(nullptr), target(FTargetData()), currentState(EActionState::Start), previousState(EActionState::Start)
{
#if WITH_EDITOR
	SetFolderPath(TEXT("Other/Controllers"));
#endif
	AActor::SetIsTemporarilyHiddenInEditor(true);
	PrimaryActorTick.TickInterval = 0.3;
	minCaptureDistance = 650;
	targetRange = 15000;

	canCheckForEnemies = true;
	currentGameMode = EGameModeType::Domination;

	// Create and configure perception components
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	sightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	// Set up sight config for AI perception
	sightConfig->SightRadius = targetRange / 2;
	sightConfig->LoseSightRadius = targetRange * 1.1;
	sightConfig->PeripheralVisionAngleDegrees = 360.0f;

	// This section is important, as without setting at least bDetectNeutrals to true, the AI will never perceive anything
	// Still not tried to set this up correctly at all
	sightConfig->DetectionByAffiliation.bDetectEnemies = true;
	sightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	sightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	PerceptionComponent->SetDominantSense(sightConfig->GetSenseImplementation());
	PerceptionComponent->ConfigureSense(*sightConfig);
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADroneBaseAI::TargetPerceptionUpdated);
}

void ADroneBaseAI::CheckLastLocation()
{
	if (lastLocation == mDroneLocation && !CompareState(EActionState::DefendingObjective) && !CompareState(EActionState::CapturingObjective))
	{
		GetDrone()->Respawn();
	}
	lastLocation = FVector::ZeroVector;
}

void ADroneBaseAI::SetCurrentState(EActionState val)
{
	if (currentState != val)
	{
		SetPreviousState(currentState);
		currentState = val;

		UE_LOG(LogDroneRPG, Log, TEXT("Drone %s state changed from %s to %s"), *GetDrone()->GetDroneName(), *GetStateString(GetPreviousState()), *GetStateString(GetCurrentState()));
	}
}

void ADroneBaseAI::SetPreviousState(EActionState val)
{
	previousState = val;
}

void ADroneBaseAI::SetCurrentGameMode(EGameModeType val)
{
	currentGameMode = val;
}

FString ADroneBaseAI::GetStateString(EActionState state)
{
	switch (state)
	{
	case EActionState::MovingToObjective:
		return "SearchingForObjective";
	case EActionState::AttackingTarget:
		return "AttackingTarget";
	case EActionState::CapturingObjective:
		return "CapturingObjective";
	case EActionState::DefendingObjective:
		return "DefendingObjective";
	case EActionState::ReturningToBase:
		return "ReturningToBase";
	case EActionState::Start:
		return "Start";
	}
	return "Unknown";
}

void ADroneBaseAI::SetTarget(const FTargetData& inTarget)
{
	target = inTarget;

	if (GetCurrentState() == EActionState::MovingToObjective)
	{
		SetCurrentState(EActionState::AttackingTarget);
	}
}

bool ADroneBaseAI::CompareState(EActionState state)
{
	return GetCurrentState() == state;
}

// ReSharper disable once CppPassValueParameterByConstReference
void ADroneBaseAI::TargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	ADroneRPGCharacter* character = Cast<ADroneRPGCharacter>(Actor);

	if (Stimulus.WasSuccessfullySensed())
	{
		if ((!GetTarget().IsValid() || !GetTarget().IsAlive()) && IsValid(character))
		{
			if (character->GetTeam() != GetDrone()->GetTeam())
			{
				SetTarget(mCreateTargetData(character));
			}
		}
	}
	else	if (GetTarget() == Actor)
	{
		SetTarget(mCreateTargetData(NULL));
		// TODO based on state, follow or stay still
		//MoveToLocation(Stimulus.StimulusLocation);
	}
}

void ADroneBaseAI::FindSuitableObjective()
{
	// Set objective to NULL, as it may still be set from a previous Calculate Objective
	targetObjective = NULL;

	// Calculate the game mode and decide what actions to perform
	switch (GetCurrentGameMode())
	{
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
	}
}

void ADroneBaseAI::FindObjective()
{
	TArray<AObjective*> objectives = mGetActorsInWorld<AObjective>(GetWorld());

	mShuffleArray<AObjective*>(objectives);

	for (AObjective* objective : objectives)
	{
		// Check if we don't already control the objective, if we do, then we'll be picking one to defend later
		if (!objective->HasCompleteControl(GetDrone()->GetTeam()))
		{
			targetObjective = objective;
			break;
		}
	}

	// If the objective is null and we found any objectives, then find one to defend
	if (targetObjective == NULL && objectives.Num() > 0)
	{
		targetObjective = UFunctionLibrary::GetRandomObject<AObjective*>(objectives);
		MoveToObjective();
	}
	// If we have an objective, then it wasn't claimed by this team, so head towards it
	else if (targetObjective != NULL)
	{
		MoveToObjective();
	}
}


void ADroneBaseAI::DroneAttacked(AActor* attacker)
{
	// If we don't have a target OR we have one but it's different to the one we have, then target it
	ADroneRPGCharacter* droneAttacker = Cast<ADroneRPGCharacter>(attacker);

	// We have no target
	if (GetTarget() == NULL)
	{
		SetTarget(mCreateTargetData(droneAttacker));
	}
	else if (GetTarget() != droneAttacker)
	{
		// Something else has attacked us and our current target is out of line of sight or dead
		if (!GetTarget().IsAlive())
		{
			SetTarget(mCreateTargetData(droneAttacker));
		}
	}
}

void ADroneBaseAI::BeginPlay()
{
	Super::BeginPlay();

	// We need to bind to the Objective Claimed method on all the objectives, so we can track when they get taken
	// This allows the bot to move an objective as an enemy tries to take it
	for (AObjective* objective : mGetActorsInWorld<AObjective>(GetWorld()))
	{
		// Bind to the Objective Claimed method on the objective
		objective->OnObjectiveClaimed.AddDynamic(this, &ADroneBaseAI::ObjectiveTaken);
	}
}

void ADroneBaseAI::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	if (!Result.IsSuccess() && (Result.Code == EPathFollowingResult::Invalid || Result.Code == EPathFollowingResult::Blocked))
	{
		SetCurrentState(EActionState::Start);
		SetTargetObjective(NULL);
	}
	else if (CompareState(EActionState::MovingToObjective))
	{
		SetCurrentState(EActionState::CapturingObjective);
	}
}

void ADroneBaseAI::ObjectiveTaken(AObjective* objective) {
	bool hasControl = objective->HasCompleteControl(GetDrone()->GetTeam());

	// Check if the objective isn't owned by us, if it is we don't care!
	if (!hasControl && !CompareState(EActionState::CapturingObjective))
	{
		SetCurrentState(EActionState::CapturingObjective);
		targetObjective = objective;
	}
	else if (hasControl && CompareState(EActionState::CapturingObjective) && targetObjective == objective)
	{
		SetCurrentState(EActionState::MovingToObjective);
	}
}

AActor* ADroneBaseAI::FindEnemyTarget(float distance)
{
	//TArray<ADroneRPGCharacter*> drones = mGetEnemiesInRadius(distance, mDroneLocation, GetDrone()->GetTeam()); TODO improve this 

	//if (drones.Num() > 0) {
	//	mShuffleArray<ADroneRPGCharacter*>(drones);
	//	return UFunctionLibrary::GetRandomObject<ADroneRPGCharacter*>(drones);
	//}

	return NULL;
}

void ADroneBaseAI::FindTarget()
{
	//targetObjective = FindEnemyTarget();
	//target = Cast<ADroneRPGCharacter>(targetObjective);
	//SetCurrentState(EActionState::AttackingTarget);
}

void ADroneBaseAI::RotateToFace()
{
	FVector targetLocation;
	FRotator targetRotation;

	// If we have a target, turn to face it
	if (IsTargetValid())
	{
		targetLocation = GetPredictedLocation(target);
	}
	// Otherwise, if we have an objective to head to, face that
	else if (targetObjective != NULL)
	{
		targetLocation = mObjectiveLocation;
	}

	// If we set a location, then calculate a loot at rotation
	if (!targetLocation.IsNearlyZero())
	{
		// Calculate the angle to look at our target

		lookAt = UKismetMathLibrary::FindLookAtRotation(mDroneLocation, targetLocation);
		lookAt.Pitch = mDroneRotation.Pitch;
		lookAt.Roll = mDroneRotation.Roll;
		targetRotation = lookAt;
	}
	else
	{
		targetRotation = GetDrone()->GetVelocity().Rotation();
		targetRotation.Pitch = mDroneRotation.Pitch;
		targetRotation.Roll = mDroneRotation.Roll;
	}

	// Update the rotation
	//GetCharacter()->SetActorRotation(FMath::Lerp(mDroneRotation, targetRotation, 0.50f));
	GetCharacter()->SetActorRotation(targetRotation);
}

void ADroneBaseAI::GetNextVisibleTarget()
{
	TArray<AActor*> actorsSeen;
	PerceptionComponent->GetCurrentlyPerceivedActors(sightConfig->GetSenseImplementation(), actorsSeen);

	for (auto seen : actorsSeen)
	{
		ADroneRPGCharacter* other = Cast<ADroneRPGCharacter>(seen);

		if (IsValid(other) && other->GetTeam() != GetDrone()->GetTeam() && other->GetHealthComponent()->IsAlive())
		{
			SetTarget(mCreateTargetData(other));
			break;
		}
	}
}

void ADroneBaseAI::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// We don't do anything if we're dead!
	if (GetDrone()->GetHealthComponent()->IsAlive())
	{
		if (!CompareState(EActionState::ReturningToBase) && !GetDrone()->GetHealthComponent()->IsHealthy())
		{
			SetCurrentState(EActionState::ReturningToBase);
			ReturningToBase();
		}

		if (!IsTargetValid())
		{
			SetTarget(mCreateTargetData(NULL));
			GetNextVisibleTarget();
		}

		if (IsTargetValid())
		{
			if (!CompareState(EActionState::ReturningToBase))
			{
				EvadingDamage();
			}

			ShootTargetIfValid();
		}

		if (lastLocation.IsZero())
		{
			lastLocation = mDroneLocation;
			mSetTimer(TimerHandle_CheckLastLocation, &ADroneBaseAI::CheckLastLocation, 15.0f);
		}

		PerformActions();

		RotateToFace();
	}
	else if (!CompareState(EActionState::MovingToObjective))
	{
		// If this is hit, then we've likely died and need to reset our state!
		SetCurrentState(EActionState::Start);
		StopMovement();
	}
}

void ADroneBaseAI::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	//GetDrone()->GetCameraBoom()->TargetArmLength = GetDrone()->GetWeapon()->GetRange() * 1.3;
	GetDrone()->GetCameraBoom()->TargetArmLength = 10000;
}

void ADroneBaseAI::PerformActions()
{
	// Determine Action
	// Move to uncontrolled objective, hook into objective state, in case it gets taken by allies prior to arrival
	// IF attacked / hostile seen before reaching objective, engage in standard combat
	// Once objective reached, Stay and defend objective
	// Once objective captured, seek out new objective

	// Do state machine things!
	switch (GetCurrentState())
	{
	case EActionState::Start:
		FindSuitableObjective();
		break;
	case EActionState::MovingToObjective:
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
	case EActionState::ReturningToBase:
		// Do Nothing
		break;
	default:
		// Something went wrong!!
		mAddOnScreenDebugMessage("I have no idea what I'm supposed to be doing!");
		break;
	}
}

void ADroneBaseAI::MoveToObjective()
{
	float distance;

	AKeyActor* keyActor = Cast<AKeyActor>(targetObjective);

	if (keyActor != NULL)
	{
		distance = keyActor->GetSize() * 0.7f;
	}
	else
	{
		distance = 600;
	}

	FNavLocation loc;
	mRandomReachablePointInRadius(targetObjective->GetActorLocation(), distance, loc);
	MoveToLocation(loc);
	SetCurrentState(EActionState::MovingToObjective);
}

void ADroneBaseAI::DefendingObjective()
{
	// Have we gone too far from our objective? If so move closer
	if (mDist(mDroneLocation, mObjectiveLocation) >= minCaptureDistance)
	{
		MoveToObjective();
	}
}

void ADroneBaseAI::ReturningToBase()
{
	UE_LOG(LogDroneRPG, Log, TEXT("Drone %s ReturningToBase"), *GetDrone()->GetDroneName());

	ARespawnPoint* respawnPoint = GetDrone()->GetRespawnPoint();
	// TODO fix this, drone is just idle during this
	FVector loc = respawnPoint->GetActorLocation();
	FVector locA = GetDrone()->GetActorLocation();
	EPathFollowingRequestResult::Type result = MoveToLocation(loc, respawnPoint->GetSize() / 2);

	if (result == EPathFollowingRequestResult::Type::Failed)
	{
		UE_LOG(LogDroneRPG, Log, TEXT("Drone %s ReturningToBase failed move"), *GetDrone()->GetDroneName());
	}
}

bool ADroneBaseAI::ShootTargetIfValid()
{
	bool result = false;
	if (IsTargetValid())
	{
		// Attempt to attack the target, don't move to the target if we can't see it
		AttackTarget(target);
		result = true;
	}
	return result;
}

void ADroneBaseAI::EvadingDamage()
{
	if (GetDrone()->GetVelocity().IsNearlyZero())
	{
		int32 closeDistance = GetDrone()->GetWeapon()->GetRange();
		int32 count = 0;
		FNavLocation loc;
		mRandomReachablePointInRadius(GetDrone()->GetActorLocation(), closeDistance, loc);

		double dist = mDist(GetTarget().GetActorLocation(), loc);

		while (dist <= closeDistance && dist > closeDistance * 0.8 && !CanSee(GetTarget(), loc) && count < 20)
		{
			mRandomReachablePointInRadius(GetDrone()->GetActorLocation(), closeDistance, loc);
			dist = mDist(GetTarget().GetActorLocation(), loc);
			count++;
		}
		MoveToLocation(loc);
	}
}

void ADroneBaseAI::AttackingTarget()
{
	// Check the target is still valid
	if (!IsTargetValid())
	{
		if (IsValid(targetObjective)) {
			SetCurrentState(EActionState::MovingToObjective);
		}
		else
		{
			SetCurrentState(EActionState::Start);			
		}
	}
	// Shoot the current target
	else
	{
		AttackTarget(GetTarget().combatantComponent->GetOwner());
	}
}

FHitResult ADroneBaseAI::LineTraceToLocation(const FVector& startLoc, const FVector& endLocation)
{
	FHitResult hit;
	FCollisionQueryParams params;
	params.AddIgnoredActor(GetCharacter());

	GetWorld()->LineTraceSingleByChannel(hit, startLoc, endLocation, ECC_Pawn, params);

	return hit;
}

bool ADroneBaseAI::CanSee(AActor* other, const FVector& startLoc)
{
	FHitResult hit = LineTraceToLocation(startLoc, other->GetActorLocation());

	// Do we have line of sight to our target?
	if (hit.GetActor() == other)
	{
		return true;
	}
	return false;
}

FVector ADroneBaseAI::GetPredictedLocation(AActor* actor)
{
	float time = mDist(mDroneLocation, actor->GetActorLocation()) / ADroneProjectile::Default_Initial_Speed;
	//TODO Add in inaccuracy to AI here. Code already done
	//time = FMath::RandRange(time * 0.9f, time * 1.1f);
	return actor->GetActorLocation() + (actor->GetVelocity() * time);
}

void ADroneBaseAI::AttackTarget(AActor* targetToAttack)
{
	// TODO Move to weapon and change to use a listener
	FireShot(lookAt.Vector());
}

bool ADroneBaseAI::IsTargetValid()
{
	if (GetTarget().IsValid() && GetTarget().IsAlive()
		// TODO figure out a better way to handle range checks. Ignoring for now as LOS is more important
		&& FVector::Dist(GetTarget().GetActorLocation(), mDroneLocation) <= GetDrone()->GetWeapon()->GetRange())
	{
		return true;
	}

	return false;
}

void ADroneBaseAI::CapturingObjective()
{
	AObjective* currentObjective = Cast<AObjective>(targetObjective);

	// Have we gone too far from our objective? If so move closer
	if (mDist(mDroneLocation, mObjectiveLocation) >= minCaptureDistance)
	{
		MoveToObjective();
	}
	// Have we claimed the current objective?
	else if (currentObjective != NULL && currentObjective->HasCompleteControl(GetDrone()->GetTeam()))
	{
		SetCurrentState(EActionState::MovingToObjective);
	}
}

ADroneRPGCharacter* ADroneBaseAI::GetDrone()
{
	return Cast<ADroneRPGCharacter>(GetCharacter());
}

void ADroneBaseAI::FireShot(const FVector& FireDirection)
{
	GetDrone()->GetWeapon()->FireShot(FireDirection);
}
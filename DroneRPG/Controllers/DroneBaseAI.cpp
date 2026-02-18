#pragma once
#include "DroneBaseAI.h"
#include "DroneRPG/Components/HealthComponent.h"
#include "DroneRPG/DroneRPG.h"
#include "DroneRPG/DroneRPGCharacter.h"
#include "DroneRPG/GameModes/DroneRPGGameMode.h"
#include "DroneRPG/LevelActors/Objective.h"
#include "DroneRPG/LevelActors/RespawnPoint.h"
#include "DroneRPG/Utilities/Enums.h"
#include "DroneRPG/Utilities/FunctionLibrary.h"
#include "DroneRPG/Weapons/Weapon.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include <Kismet/KismetMathLibrary.h>

#define  mObjectiveLocation targetObjective->GetActorLocation()

ADroneBaseAI::ADroneBaseAI(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), lookAt(), lastLocation(), isFiring(false), currentState(EActionState::Start), previousState(EActionState::Start), targetObjective(nullptr), target(FTargetData())
{
	AActor::SetIsTemporarilyHiddenInEditor(true);
	drawDebug = false;

	PrimaryActorTick.TickInterval = 0.3;
	minCaptureDistance = 100;

	canCheckForEnemies = true;
	currentGameMode = EGameModeType::Domination;

	// Create and configure perception components
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	sightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	// Set up sight config for AI perception
	sightConfig->PeripheralVisionAngleDegrees = 270;

	// This section is important, as without setting at least bDetectNeutrals to true, the AI will never perceive anything
	// Still not tried to set this up correctly at all
	sightConfig->DetectionByAffiliation.bDetectEnemies = true;
	sightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	sightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	sightConfig->AutoSuccessRangeFromLastSeenLocation = 0;

	PerceptionComponent->SetDominantSense(sightConfig->GetSenseImplementation());
	PerceptionComponent->ConfigureSense(*sightConfig);
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADroneBaseAI::TargetPerceptionUpdated);

	static ConstructorHelpers::FObjectFinder<UEnvQuery> PlayerLocationQueryObj(TEXT("/ Script / AIModule.EnvQuery'/Game/TopDownCPP/EQS/GetEmptyPointsQuery.GetEmptyPointsQuery'"));
	static ConstructorHelpers::FObjectFinder<UEnvQuery> PlayerEvadeQueryObj(TEXT("/Script/AIModule.EnvQuery'/Game/TopDownCPP/EQS/GetEvasionPoints.GetEvasionPoints'"));

	if (PlayerLocationQueryObj.Succeeded() && PlayerEvadeQueryObj.Succeeded())
	{
		FindLocationEmptyLocationRequest = FEnvQueryRequest(PlayerLocationQueryObj.Object, this);
		FindEvadeLocationRequest = FEnvQueryRequest(PlayerEvadeQueryObj.Object, this);
	}
	else
	{
		UE_LOG(LogDroneAI, Error, TEXT("Failed to find EQS EnvQuery asset."));
	}
}

void ADroneBaseAI::CheckLastLocation()
{
	if (lastLocation == mDroneLocation && !CompareState(EActionState::DefendingObjective) && !CompareState(EActionState::CapturingObjective))
	{
		GetDrone()->Respawn();
		SetCurrentState(EActionState::Start);
		SetTarget(FTargetData());
	}
	lastLocation = FVector::ZeroVector;
}

void ADroneBaseAI::SetCurrentState(EActionState val)
{
	if (currentState != val)
	{
		SetPreviousState(currentState);
		currentState = val;

		UE_LOG(LogDroneAI, Log, TEXT("%s state changed from %s to %s"), *GetDrone()->GetDroneName(), *GetStateString(GetPreviousState()), *GetStateString(GetCurrentState()));
	}
}

void ADroneBaseAI::SetPreviousState(EActionState val)
{
	previousState = val;
}

void ADroneBaseAI::SetTargetObjective(AObjective* val)
{
	targetObjective = val;

	if (IsValid(targetObjective))
	{
		minCaptureDistance = targetObjective->GetSize() * .7;
		targetObjective->OnObjectiveClaimed.AddUniqueDynamic(this, &ADroneBaseAI::ObjectiveTaken);
	}
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
		return "MovingToObjective";
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

void ADroneBaseAI::OnTargetUnitDied(UCombatantComponent* inKiller)
{
	UE_LOG(LogDroneAI, Log, TEXT("%s's target ( %s) died "), *GetDrone()->GetDroneName(), *GetTarget().GetCombatantName());
	SetTarget(FTargetData());

	if (CompareState(EActionState::AttackingTarget))
	{
		FindSuitableObjective();
	}
}

void ADroneBaseAI::SetTarget(const FTargetData& inTarget)
{
	if (target.isSet)
	{
		target.healthComponent->OnUnitDied.RemoveAll(this);
	}

	target = inTarget;

	if (target.isSet)
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s's target set %s"), *GetDrone()->GetDroneName(), *GetTarget().GetCombatantName());
		target.healthComponent->OnUnitDied.AddUniqueDynamic(this, &ADroneBaseAI::OnTargetUnitDied);
	}

	if (target.isSet && !CompareState(EActionState::AttackingTarget))
	{
		SetCurrentState(EActionState::AttackingTarget);
		StopMovement();
	}
}

bool ADroneBaseAI::CompareState(EActionState state)
{
	return GetCurrentState() == state;
}

void ADroneBaseAI::ActorSeen(AActor* Actor)
{
	if (!IsTargetValid())
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s's actor seen"), *GetDrone()->GetDroneName());
		FTargetData data = mCreateTargetData(Actor);

		if (IsTargetValid(data))
		{
			SetTarget(data);
		}
	}
}

void ADroneBaseAI::LostSightOfActor(AActor* Actor, const FVector& lastSeenLocation)
{
	if (GetTarget() == Actor)
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s's lost sight of  target %s"), *GetDrone()->GetDroneName(), *GetTarget().GetCombatantName());
		if (CompareState(EActionState::AttackingTarget))
		{
			MoveToLocation(lastSeenLocation);
		}
		else if (!GetNextVisibleTarget())
		{
			SetTarget(FTargetData());
		}
	}
}

// ReSharper disable once CppPassValueParameterByConstReference
void ADroneBaseAI::TargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			ActorSeen(Actor);
		}
		else
		{
			LostSightOfActor(Actor, Stimulus.StimulusLocation);
		}
	}
}

void ADroneBaseAI::DrawNavigationDebug(const FVector& location, FColor colour)
{
	if (drawDebug)
	{
		DrawDebugLine(GetWorld(), mDroneLocation, location, colour, false, 0.5);
		DrawDebugSphere(GetWorld(), location, 100, 10, colour, false, 0.5);
	}
}

void ADroneBaseAI::FailedToMoveToLocation(const FVector& location)
{
	DrawNavigationDebug(location, FColor::Red);
	if (IsNotMoving())
	{
		//UE_LOG(LogDroneRPG, Warning, TEXT("Drone %s failed move to %s"), *GetDrone()->GetDroneName(), *location.ToString());
	}
	else
	{
		//	UE_LOG(LogDroneRPG, Warning, TEXT("Drone %s moving to new location, whilst moving during state %s"), *GetDrone()->GetDroneName(), *GetStateString(GetCurrentState()));
	}
}

void ADroneBaseAI::MoveRequestFinished(TSharedPtr<FEnvQueryResult> Result)
{
	if (Result->IsSuccessful())
	{
		FVector location = Result->GetItemAsLocation(0);
		EPathFollowingRequestResult::Type result = MoveToLocation(location, acceptanceRadius);

		if (result == EPathFollowingRequestResult::Type::Failed)
		{
			isMoving = false;
			FailedToMoveToLocation(location);
		}
		else
		{
			isMoving = true;
			DrawNavigationDebug(location, FColor::Green);
		}
	}
	else
	{
		isMoving = false;
		FailedToMoveToLocation(queryLocation);
	}
	isRequestingMovement = false;
}

ADroneRPGGameMode* ADroneBaseAI::GetGameMode()
{
	if (!IsValid(gameMode))
	{
		gameMode = Cast<ADroneRPGGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	}
	return gameMode;
}

void ADroneBaseAI::StopMovement()
{
	Super::StopMovement();
	isMoving = false;
}

void ADroneBaseAI::FindSuitableObjective()
{
	// Set objective to NULL, as it may still be set from a previous Calculate Objective
	targetObjective = nullptr;

	// Calculate the game mode and decide what actions to perform
	switch (GetCurrentGameMode())
	{
	case EGameModeType::TeamDeathMatch:
		// We only need to find enemies in the team death match
		UE_LOG(LogDroneAI, Log, TEXT("%s FindSuitableObjective FindTarget"), *GetDrone()->GetDroneName());
		FindTarget();
		break;
	case EGameModeType::Domination:
	case EGameModeType::AttackDefend:
	case EGameModeType::Payload:
	case EGameModeType::Hardpoint:
		// In all other cases we will need to find an objective TODO: this may need to change in game modes such as AttackDefend
		UE_LOG(LogDroneAI, Log, TEXT("%s FindSuitableObjective FindObjective"), *GetDrone()->GetDroneName());
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

	AObjective* closest = nullptr;

	for (AObjective* objective : objectives)
	{
		// Check if we don't already control the objective, if we do, then we'll be picking one to defend later
		if (!objective->HasCompleteControl(GetDrone()->GetTeam()))
		{
			if (!IsValid(closest))
			{
				closest = objective;
			}
			else if (FVector::Dist(closest->GetActorLocation(), mDroneLocation) > FVector::Dist(objective->GetActorLocation(), mDroneLocation))
			{
				closest = objective;
			}
		}
	}

	// If we have an objective, then it wasn't claimed by this team, so head towards it
	if (IsValid(closest))
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s moving to closest objective"), *GetDrone()->GetDroneName());
		SetTargetObjective(closest);
		MoveToObjective();
	}
	// If the objective is null and we found any objectives, then find one to defend
	else if (objectives.Num() > 0)
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s moving to random objective"), *GetDrone()->GetDroneName());
		SetTargetObjective(UFunctionLibrary::GetRandomObject<AObjective*>(objectives));
		MoveToObjective();
	}
	else
	{
		UE_LOG(LogDroneRPG, Error, TEXT("Drone %s failed to find objectives"), *GetDrone()->GetDroneName());
	}
}

void ADroneBaseAI::DroneAttacked(AActor* attacker)
{
	if (!CompareState(EActionState::ReturningToBase) && !GetDrone()->GetHealthComponent()->IsHealthy())
	{
		SetCurrentState(EActionState::ReturningToBase);
		ReturningToBase();
	}
	// We have no target
	else if (!IsTargetValid())
	{
		FTargetData data = mCreateTargetData(attacker);

		if (IsTargetValid(data))
		{
			UE_LOG(LogDroneAI, Log, TEXT("%s hit by hostile %s"), *GetDrone()->GetDroneName(), *GetTarget().GetCombatantName());
			SetTarget(data);
		}
	}
}

void ADroneBaseAI::BeginPlay()
{
	Super::BeginPlay();

#if WITH_EDITOR
	SetFolderPath(TEXT("Other/Controllers"));
#endif
}

void ADroneBaseAI::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);
	isMoving = false;

	if (Result.IsSuccess() && CompareState(EActionState::MovingToObjective) && IsValid(GetTargetObjective()))
	{
		// TODO Check we have made it to our objective
		if (!GetTargetObjective()->HasCompleteControl(GetDrone()->GetTeam()))
		{
			SetCurrentState(EActionState::CapturingObjective);
		}
		else
		{
			UE_LOG(LogDroneAI, Log, TEXT("%s OnMoveCompleted FindSuitableObjective"), *GetDrone()->GetDroneName());
			FindSuitableObjective();
		}
	}
}

void ADroneBaseAI::SetFiringState(bool firingState)
{
	if (GetDrone()->GetWeapon()->IsActive() != firingState)
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s shooting at %s"), *GetDrone()->GetDroneName(), *GetTarget().GetCombatantName());
		GetDrone()->GetWeapon()->SetActive(firingState);
	}
}

void ADroneBaseAI::ObjectiveTaken(AObjective* objective)
{
	bool hasControl = objective->HasCompleteControl(GetDrone()->GetTeam());

	// Check if the objective isn't owned by us, if it is we don't care!
	if (!hasControl && (CompareState(EActionState::Start) || CompareState(EActionState::DefendingObjective)))
	{
		SetTargetObjective(objective);

		UE_LOG(LogDroneAI, Log, TEXT("%s ObjectiveTaken MoveToObjective"), *GetDrone()->GetDroneName());
		MoveToObjective();
	}
	else if (hasControl && (CompareState(EActionState::CapturingObjective) || CompareState(EActionState::MovingToObjective)) && GetTargetObjective() == objective)
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s ObjectiveTaken FindSuitableObjective"), *GetDrone()->GetDroneName());
		FindSuitableObjective();
	}
}

AActor* ADroneBaseAI::FindEnemyTarget()
{
	TArray<AActor*> actors;

	for (auto combatant : GetGameMode()->GetCombatants())
	{
		if (combatant->GetTeam() != GetDrone()->GetTeam())
		{
			actors.Add(combatant->GetOwner());
		}
	}

	if (actors.Num() > 0)
	{
		return mGetClosestActorInArray(actors, mDroneLocation);
	}

	return nullptr;
}

void ADroneBaseAI::FindTarget()
{
	//targetObjective = FindEnemyTarget();
	//target = Cast<ADroneRPGCharacter>(targetObjective);
	//SetCurrentState(EActionState::AttackingTarget);
}

void ADroneBaseAI::RotateToFace(float DeltaSeconds)
{
	FVector targetLocation;

	// If we have a target, turn to face it
	if (GetTarget().isSet && GetTarget().IsAlive())
	{
		targetLocation = GetPredictedLocation(GetTarget());
	}
	// Otherwise, if we have an objective to head to, face that
	else if (IsValid(GetTargetObjective()))
	{
		targetLocation = mObjectiveLocation;
	}

	lookAt = UKismetMathLibrary::FindLookAtRotation(GetPawn()->GetActorLocation(), targetLocation);

	lookAt.Pitch = 0;
	lookAt.Roll = 0;

	GetPawn()->SetActorRotation(FMath::RInterpTo(GetPawn()->GetActorRotation(), lookAt, DeltaSeconds, 8.0f));
}

bool ADroneBaseAI::GetNextVisibleTarget()
{
	bool result = false;
	TArray<AActor*> actorsSeen;
	PerceptionComponent->GetCurrentlyPerceivedActors(sightConfig->GetSenseImplementation(), actorsSeen);

	for (auto seen : actorsSeen)
	{
		FTargetData data = mCreateTargetData(seen);

		if (IsTargetValid(data))
		{
			SetTarget(data);
			result = true;
			break;
		}
	}

	return result;
}

void ADroneBaseAI::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// We don't do anything if we're dead!
	if (GetDrone()->GetHealthComponent()->IsAlive())
	{
		RotateToFace(DeltaSeconds);

		if (IsTargetValid())
		{
			if (!CompareState(EActionState::ReturningToBase))
			{
				EvadingDamage();
			}

			SetFiringState(true);
		}
		else
		{
			SetFiringState(false);			
		}

		if (lastLocation.IsZero())
		{
			lastLocation = mDroneLocation;
			mSetTimer(TimerHandle_CheckLastLocation, &ADroneBaseAI::CheckLastLocation, 15.0f);
		}

		PerformActions();
	}
	else
	{
		// If this is hit, then we've likely died and need to reset our state!
		SetCurrentState(EActionState::Start);
		StopMovement();
		SetFiringState(false);
	}
}

void ADroneBaseAI::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	//GetDrone()->GetCameraBoom()->TargetArmLength = 1500;

	float range = 4000;

	sightConfig->SightRadius = range * .8;
	sightConfig->LoseSightRadius = range;
	PerceptionComponent->RequestStimuliListenerUpdate();
	acceptanceRadius = 100;
}

void ADroneBaseAI::MovingToObjective()
{
	if (IsValid(GetTargetObjective()))
	{
		if (IsNotMoving())
		{
			UE_LOG(LogDroneAI, Log, TEXT("%s MovingToObjective IsNotMoving"), *GetDrone()->GetDroneName());
			MoveToObjective();
		}
	}
	else
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s MovingToObjective FindSuitableObjective"), *GetDrone()->GetDroneName());
		FindSuitableObjective();
	}
}

void ADroneBaseAI::PerformActions()
{
	// Do state machine things!
	switch (GetCurrentState())
	{
	case EActionState::Start:
		UE_LOG(LogDroneAI, Log, TEXT("%s Start FindSuitableObjective"), *GetDrone()->GetDroneName());
		FindSuitableObjective();
		break;
	case EActionState::MovingToObjective:
		MovingToObjective();
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
		ReturningToBase();
		break;
	default:
		// Something went wrong!!
		mAddOnScreenDebugMessage("I have no idea what I'm supposed to be doing!");
		break;
	}
}

void ADroneBaseAI::RunMoveQuery(FEnvQueryRequest& query, const FVector& inLocation, float inGetRange, const FString& source)
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

	if (!isRequestingMovement && NavSys)
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s RunMoveQuery %s"), *GetDrone()->GetDroneName(), *source);
		isRequestingMovement = true;

		FNavLocation Projected;
		NavSys->ProjectPointToNavigation(inLocation, Projected, FVector(300, 300, 300), nullptr, nullptr);

		queryLocation = Projected.Location;
		query.SetFloatParam("Radius", inGetRange * .95f);
		query.Execute(EEnvQueryRunMode::RandomBest5Pct, this, &ADroneBaseAI::MoveRequestFinished);
	}
}

void ADroneBaseAI::RunMoveQuery(const FVector& location, double radius, const FString& source)
{
	RunMoveQuery(FindLocationEmptyLocationRequest, location, radius, source);
}

void ADroneBaseAI::MoveToObjective()
{
	if (IsValid(GetTargetObjective()) && IsNotMoving())
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s moving to objective"), *GetDrone()->GetDroneName());
		RunMoveQuery(mObjectiveLocation, minCaptureDistance, "MoveToObjective");
		SetCurrentState(EActionState::MovingToObjective);
	}
}

void ADroneBaseAI::DefendingObjective()
{
	// Have we gone too far from our objective? If so move closer
	if (mDist(mDroneLocation, mObjectiveLocation) >= minCaptureDistance)
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s DefendingObjective MoveToObjective"), *GetDrone()->GetDroneName());
		MoveToObjective();
	}
}

void ADroneBaseAI::ReturningToBase()
{
	if (GetDrone()->GetHealthComponent()->IsHealthy())
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s fully healed"), *GetDrone()->GetDroneName());
		SetCurrentState(EActionState::Start);
	}
	else if (IsNotMoving())
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s moving to respawn"), *GetDrone()->GetDroneName());
		ARespawnPoint* respawnPoint = GetDrone()->GetRespawnPoint();
		RunMoveQuery(respawnPoint->GetActorLocation(), respawnPoint->GetSize(), "ReturningToBase");
	}
}

float ADroneBaseAI::GetWeaponRange()
{
	return GetDrone()->GetWeapon()->GetRange();
}

void ADroneBaseAI::EvadingDamage()
{
	if (IsNotMoving())
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s evading damage"), *GetDrone()->GetDroneName());
		/*	if (CompareState(EActionState::AttackingTarget) || CompareState(EActionState::MovingToObjective))
			{*/
		RunMoveQuery(FindEvadeLocationRequest, GetTarget().GetActorLocation(), GetWeaponRange(), "EvadingDamage");
		//}
		//else if (IsValid(GetTargetObjective()))
		//{
		//	RunMoveQuery(FindEvadeLocationRequest, GetTargetObjective()->GetActorLocation(), GetTargetObjective()->GetSize());
		//}
	}
}

bool ADroneBaseAI::IsNotMoving()
{
	return !isRequestingMovement && !isMoving;
}

bool ADroneBaseAI::IsTargetInWeaponRange()
{
	return IsTargetInWeaponRange(GetTarget());
}

bool ADroneBaseAI::IsTargetInWeaponRange(const FTargetData& targetToCheck)
{
	return FVector::Dist(targetToCheck.GetActorLocation(), mDroneLocation) <= GetWeaponRange();
}

void ADroneBaseAI::AttackingTarget()
{
	// Are we in range?
	if (IsTargetValid() && !IsTargetInWeaponRange() && IsNotMoving())
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s target out of range"), *GetDrone()->GetDroneName());
		RunMoveQuery(FindEvadeLocationRequest, GetTarget().GetActorLocation(), GetWeaponRange(), "AttackingTarget");
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
	return hit.GetActor() == other;
}

FVector ADroneBaseAI::GetPredictedLocation(AActor* actor)
{
	float time = mDist(mDroneLocation, actor->GetActorLocation()) / GetDrone()->GetWeapon()->GetProjectileSpeed();
	//TODO Add in inaccuracy to AI here. Code already done
	//time = FMath::RandRange(time * 0.9f, time * 1.1f);
	return actor->GetActorLocation() + (actor->GetVelocity() * time);
}

bool ADroneBaseAI::IsTargetValid(FTargetData& data)
{
	//&& IsTargetInWeaponRange(data)
	if (data.IsValid() && data.IsAlive() && data.GetTeam() != GetDrone()->GetTeam()
		)
	{
		return true;
	}

	return false;
}

void ADroneBaseAI::CapturingObjective()
{
	// Have we gone too far from our objective? If so move closer
	if (IsNotMoving() && mDist(mDroneLocation, mObjectiveLocation) >= minCaptureDistance)
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s moving back to objective range"), *GetDrone()->GetDroneName());
		MoveToObjective();
	}
	// Have we claimed the current objective?
	else if (IsValid(GetTargetObjective()) && GetTargetObjective()->HasCompleteControl(GetDrone()->GetTeam()))
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s objective captured, finding new objective"), *GetDrone()->GetDroneName());
		FindSuitableObjective();
	}
}

ADroneRPGCharacter* ADroneBaseAI::GetDrone()
{
	if (!IsValid(droneCharacter))
	{
		droneCharacter = Cast<ADroneRPGCharacter>(GetCharacter());
	}
	return droneCharacter;
}

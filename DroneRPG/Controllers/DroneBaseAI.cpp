#pragma once
#include "DroneBaseAI.h"
#include "DroneRPG/Components/HealthComponent.h"
#include "DroneRPG/DroneRPG.h"
#include "DroneRPG/DroneRPGCharacter.h"
#include "DroneRPG/GameModes/DroneRPGGameMode.h"
#include "DroneRPG/LevelActors/RespawnPoint.h"
#include "DroneRPG/Utilities/Enums.h"
#include "DroneRPG/Utilities/FunctionLibrary.h"
#include "DroneRPG/Weapons/Weapon.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "GameFramework/Character.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include <Kismet/KismetMathLibrary.h>

#include "DroneRPG/Components/ObjectiveComponent.h"

#define  mObjectiveLocation targetObjective->GetOwner()->GetActorLocation()

ADroneBaseAI::ADroneBaseAI(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), lookAt(), lastLocation(), currentState(EActionState::Start), previousState(EActionState::Start), targetObjective(nullptr)
{
	AActor::SetIsTemporarilyHiddenInEditor(true);
	drawDebug = DRONE_DEBUG_ENABLED;

	PrimaryActorTick.TickInterval = 0.05;
	minCaptureDistance = 200;

	canCheckForEnemies = true;
	currentGameMode = EGameModeType::Domination;

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
	// If this is hit, then we've likely died and need to reset our state!
	else
	{
		SetTarget(FCombatantData());
		SetCurrentState(EActionState::Start);
		StopMovement();
		SetFiringState(false);
	}
}

void ADroneBaseAI::EvadingDamage()
{
	if (IsNotMoving())
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s evading damage"), *GetDrone()->GetCharacterName());
		RunMoveQuery(FindEvadeLocationRequest, GetTarget().GetActorLocation(), GetWeaponRange(), "EvadingDamage");
	}
}

// Region Utilities

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

ADroneRPGCharacter* ADroneBaseAI::GetDrone()
{
	if (!IsValid(droneCharacter))
	{
		droneCharacter = Cast<ADroneRPGCharacter>(GetCharacter());
	}
	return droneCharacter;
}

void ADroneBaseAI::SetCurrentState(EActionState val)
{
	if (currentState != val)
	{
		SetPreviousState(currentState);
		currentState = val;

		UE_LOG(LogDroneAI, Log, TEXT("%s state changed from %s to %s"), *GetDrone()->GetCharacterName(), *GetStateString(GetPreviousState()), *GetStateString(GetCurrentState()));
	}
}

void ADroneBaseAI::SetPreviousState(EActionState val)
{
	previousState = val;
}

void ADroneBaseAI::SetTargetObjective(UObjectiveComponent* val)
{
	targetObjective = val;

	if (IsValid(targetObjective))
	{
		minCaptureDistance = targetObjective->GetSize() * .7;
		targetObjective->OnObjectiveClaimed.AddUniqueDynamic(this, &ADroneBaseAI::ObjectiveTaken);
	}
}

bool ADroneBaseAI::CompareState(EActionState state)
{
	return GetCurrentState() == state;
}

bool ADroneBaseAI::IsNotMoving()
{
	return !isRequestingMovement && !isMoving;
}

bool ADroneBaseAI::IsTargetInWeaponRange()
{
	return IsTargetInWeaponRange(GetTarget());
}

bool ADroneBaseAI::IsTargetInWeaponRange(const FCombatantData& targetToCheck)
{
	return FVector::Dist(targetToCheck.GetActorLocation(), mDroneLocation) <= GetWeaponRange();
}

void ADroneBaseAI::SetFiringState(bool firingState)
{
	if (GetDrone()->GetWeapon()->IsActive() != firingState)
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s shooting at %s"), *GetDrone()->GetCharacterName(), *GetTarget().GetCombatantName());
		GetDrone()->GetWeapon()->SetActive(firingState);
	}
}

float ADroneBaseAI::GetWeaponRange()
{
	return GetDrone()->GetWeapon()->GetRange();
}

// Region End Utilities


// Region AI State machine

void ADroneBaseAI::SetTarget(const FCombatantData& inTarget)
{
	Super::SetTarget(inTarget);

	if (target.isSet && !CompareState(EActionState::AttackingTarget))
	{
		SetCurrentState(EActionState::AttackingTarget);
		StopMovement();
	}
	else if (!target.isSet && CompareState(EActionState::AttackingTarget))
	{
		SetCurrentState(EActionState::Start);
	}
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
		UE_LOG(LogDroneAI, Log, TEXT("%s FindSuitableObjective FindTarget"), *GetDrone()->GetCharacterName());
		FindTarget();
		break;
	case EGameModeType::Domination:
	case EGameModeType::AttackDefend:
	case EGameModeType::Payload:
	case EGameModeType::Hardpoint:
		// In all other cases we will need to find an objective TODO: this may need to change in game modes such as AttackDefend
		UE_LOG(LogDroneAI, Log, TEXT("%s FindSuitableObjective FindObjective"), *GetDrone()->GetCharacterName());
		FindObjective();
		break;
	default:
		mAddOnScreenDebugMessage("I have no idea what I'm supposed to be doing!");
		break;
	}
}

void ADroneBaseAI::FindObjective()
{
	UObjectiveComponent* closest = GetClosestUncontrolledObjective();

	// If we have an objective, then it wasn't claimed by this team, so head towards it
	if (IsValid(closest))
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s moving to closest objective"), *GetDrone()->GetCharacterName());
		SetTargetObjective(closest);
		MoveToObjective();
	}
	// If the objective is null and we found any objectives, then find one to defend
	else
	{
		TArray<UObjectiveComponent*> objectives = GetGameMode()->GetObjectives();

		if (objectives.Num() > 0)
		{
			UE_LOG(LogDroneAI, Log, TEXT("%s moving to random objective"), *GetDrone()->GetCharacterName());
			SetTargetObjective(UFunctionLibrary::GetRandomObject<UObjectiveComponent*>(objectives));
			MoveToObjective();
		}
		else
		{
			UE_LOG(LogDroneRPG, Error, TEXT("Drone %s failed to find objectives"), *GetDrone()->GetCharacterName());
		}
	}
}

void ADroneBaseAI::AttackingTarget()
{
	// Are we in range?
	if (IsTargetValid() && !IsTargetInWeaponRange() && IsNotMoving())
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s target out of range"), *GetDrone()->GetCharacterName());
		RunMoveQuery(FindEvadeLocationRequest, GetTarget().GetActorLocation(), GetWeaponRange(), "AttackingTarget");
	}
}

void ADroneBaseAI::CapturingObjective()
{
	// Have we gone too far from our objective? If so move closer
	if (IsNotMoving() && mDist(mDroneLocation, mObjectiveLocation) >= minCaptureDistance)
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s moving back to objective range"), *GetDrone()->GetCharacterName());
		MoveToObjective();
	}
	// Have we claimed the current objective?
	else if (IsValid(GetTargetObjective()) && GetTargetObjective()->HasCompleteControl(GetDrone()->GetTeam()))
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s objective captured, finding new objective"), *GetDrone()->GetCharacterName());
		FindSuitableObjective();
	}
}

void ADroneBaseAI::MoveToObjective()
{
	if (IsValid(GetTargetObjective()) && IsNotMoving())
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s moving to objective"), *GetDrone()->GetCharacterName());
		RunMoveQuery(mObjectiveLocation, minCaptureDistance, "MoveToObjective");
		SetCurrentState(EActionState::MovingToObjective);
	}
}

void ADroneBaseAI::DefendingObjective()
{
	// Have we gone too far from our objective? If so move closer
	if (mDist(mDroneLocation, mObjectiveLocation) >= minCaptureDistance)
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s DefendingObjective MoveToObjective"), *GetDrone()->GetCharacterName());
		MoveToObjective();
	}
}

void ADroneBaseAI::ReturningToBase()
{
	if (GetDrone()->GetHealthComponent()->IsHealthy())
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s fully healed"), *GetDrone()->GetCharacterName());
		SetCurrentState(EActionState::Start);
	}
	else if (IsNotMoving())
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s moving to respawn"), *GetDrone()->GetCharacterName());
		ARespawnPoint* respawnPoint = GetDrone()->GetRespawnPoint();
		RunMoveQuery(respawnPoint->GetActorLocation(), respawnPoint->GetSize(), "ReturningToBase");
	}
}

void ADroneBaseAI::RunMoveQuery(FEnvQueryRequest& query, const FVector& inLocation, float inGetRange, const FString& source)
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

	if (!isRequestingMovement && NavSys)
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s RunMoveQuery %s"), *GetDrone()->GetCharacterName(), *source);
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

void ADroneBaseAI::MovingToObjective()
{
	if (IsValid(GetTargetObjective()))
	{
		if (IsNotMoving())
		{
			UE_LOG(LogDroneAI, Log, TEXT("%s MovingToObjective IsNotMoving"), *GetDrone()->GetCharacterName());
			MoveToObjective();
		}
	}
	else
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s MovingToObjective FindSuitableObjective"), *GetDrone()->GetCharacterName());
		FindSuitableObjective();
	}
}

void ADroneBaseAI::PerformActions()
{
	// Do state machine things!
	switch (GetCurrentState())
	{
	case EActionState::Start:
		UE_LOG(LogDroneAI, Log, TEXT("%s Start FindSuitableObjective"), *GetDrone()->GetCharacterName());
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

// Region End AI State machine


// Region AI Events / Interrupts

void ADroneBaseAI::OnTargetUnitDied(UCombatantComponent* inKiller)
{
	Super::OnTargetUnitDied(inKiller);

	if (CompareState(EActionState::AttackingTarget))
	{
		FindSuitableObjective();
	}
}

void ADroneBaseAI::LostSightOfActor(AActor* Actor, const FVector& lastSeenLocation)
{
	if (GetTarget() == Actor)
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s's lost sight of  target %s"), *GetDrone()->GetCharacterName(), *GetTarget().GetCombatantName());

		if (CompareState(EActionState::AttackingTarget))
		{
			MoveToLocation(lastSeenLocation);
		}
		else if (!GetNextVisibleTarget())
		{
			SetTarget(FCombatantData());
		}
	}
}

// ReSharper disable once CppPassValueParameterByConstReference
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

void ADroneBaseAI::FailedToMoveToLocation(const FVector& location)
{
	DrawNavigationDebug(location, FColor::Red);
	if (IsNotMoving())
	{
		UE_LOG(LogDroneRPG, Warning, TEXT("Drone %s failed move to %s"), *GetDrone()->GetCharacterName(), *location.ToString());
	}
	else
	{
		UE_LOG(LogDroneRPG, Warning, TEXT("Drone %s moving to new location, whilst moving during state %s"), *GetDrone()->GetCharacterName(), *GetStateString(GetCurrentState()));
	}
}

void ADroneBaseAI::OwnerAttacked(AActor* attacker)
{
	if (!CompareState(EActionState::ReturningToBase) && !GetDrone()->GetHealthComponent()->IsHealthy())
	{
		SetCurrentState(EActionState::ReturningToBase);
		ReturningToBase();
	}
	// We have no target
	else if (!IsTargetValid())
	{
		FCombatantData data = mCreateCombatantData(attacker);

		if (IsTargetValid(data))
		{
			UE_LOG(LogDroneAI, Log, TEXT("%s hit by hostile %s"), *GetDrone()->GetCharacterName(), *GetTarget().GetCombatantName());
			SetTarget(data);
		}
	}
}

void ADroneBaseAI::CheckLastLocation()
{
	if (lastLocation == mDroneLocation && !CompareState(EActionState::DefendingObjective) && !CompareState(EActionState::CapturingObjective))
	{
		GetDrone()->Respawn();
		SetCurrentState(EActionState::Start);
		SetTarget(FCombatantData());
	}
	lastLocation = FVector::ZeroVector;
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
			UE_LOG(LogDroneAI, Log, TEXT("%s OnMoveCompleted FindSuitableObjective"), *GetDrone()->GetCharacterName());
			FindSuitableObjective();
		}
	}
}

void ADroneBaseAI::StopMovement()
{
	Super::StopMovement();
	isMoving = false;
}

void ADroneBaseAI::ObjectiveTaken(UObjectiveComponent* objective)
{
	bool hasControl = objective->HasCompleteControl(GetDrone()->GetTeam());

	// Check if the objective isn't owned by us, if it is we don't care!
	if (!hasControl && (CompareState(EActionState::Start) || CompareState(EActionState::DefendingObjective)))
	{
		SetTargetObjective(objective);

		UE_LOG(LogDroneAI, Log, TEXT("%s ObjectiveTaken MoveToObjective"), *GetDrone()->GetCharacterName());
		MoveToObjective();
	}
	else if (hasControl && (CompareState(EActionState::CapturingObjective) || CompareState(EActionState::MovingToObjective)) && GetTargetObjective() == objective)
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s ObjectiveTaken FindSuitableObjective"), *GetDrone()->GetCharacterName());
		StopMovement();
		FindSuitableObjective();
	}
}

// Region End AI Events / Interrupts
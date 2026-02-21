#include "BaseAIController.h"

#include "DroneBaseAI.h"
#include "DroneRPG/DroneRPG.h"
#include "DroneRPG/Components/ObjectiveComponent.h"
#include "DroneRPG/GameModes/DroneRPGGameMode.h"
#include "DroneRPG/Utilities/FunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

ABaseAIController::ABaseAIController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Create and configure perception components
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	sightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	// Set up sight config for AI perception
	sightConfig->PeripheralVisionAngleDegrees = 350;

	// This section is important, as without setting at least bDetectNeutrals to true, the AI will never perceive anything
	// Still not tried to set this up correctly at all
	sightConfig->DetectionByAffiliation.bDetectEnemies = true;
	sightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	sightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	sightConfig->AutoSuccessRangeFromLastSeenLocation = 0;

	PerceptionComponent->SetDominantSense(sightConfig->GetSenseImplementation());
	PerceptionComponent->ConfigureSense(*sightConfig);
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ABaseAIController::TargetPerceptionUpdated);
}

FString ABaseAIController::GetStateString(EActionState state)
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

void ABaseAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	combatant = mCreateCombatantData(InPawn);

	float range = 6000;

	sightConfig->SightRadius = range * .8;
	sightConfig->LoseSightRadius = range;
	PerceptionComponent->RequestStimuliListenerUpdate();
	acceptanceRadius = 100;
}

void ABaseAIController::BeginPlay()
{
	Super::BeginPlay();

#if WITH_EDITOR
	SetFolderPath(TEXT("Other/Controllers"));
#endif
}

void ABaseAIController::SetFiringState(bool firingState)
{
	// Not everything has a gun handle. Maybe add to combatant component?
}

void ABaseAIController::OwnerAttacked(AActor* attacker)
{
	if (!IsTargetValid())
	{
		FCombatantData data = mCreateCombatantData(attacker);

		if (IsTargetValid(data))
		{
			UE_LOG(LogDroneAI, Log, TEXT("%s hit by hostile %s"), *GetCombatantName(), *GetTarget().GetCombatantName());
			SetTarget(data);
		}
	}
}

bool ABaseAIController::IsTargetValid(FCombatantData& data)
{
	if (data.IsValid() && data.IsAlive() && data.GetTeam() != combatant.GetTeam())
	{
		return true;
	}

	return false;
}

bool ABaseAIController::GetNextVisibleTarget()
{
	bool result = false;
	TArray<AActor*> actorsSeen;
	PerceptionComponent->GetCurrentlyPerceivedActors(sightConfig->GetSenseImplementation(), actorsSeen);

	for (auto seen : actorsSeen)
	{
		FCombatantData data = mCreateCombatantData(seen);

		if (IsTargetValid(data))
		{
			SetTarget(data);
			result = true;
			break;
		}
	}

	return result;
}

void ABaseAIController::SetTarget(const FCombatantData& inTarget)
{
	if (target.isSet)
	{
		target.healthComponent->OnUnitDied.RemoveAll(this);
	}

	target = inTarget;

	if (target.isSet)
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s's target set %s"), *GetCombatantName(), *GetTarget().GetCombatantName());
		target.healthComponent->OnUnitDied.AddUniqueDynamic(this, &ABaseAIController::OnTargetUnitDied);
	}
}

ADroneRPGGameMode* ABaseAIController::GetGameMode()
{
	if (!IsValid(gameMode))
	{
		gameMode = Cast<ADroneRPGGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	}
	return gameMode;
}

void ABaseAIController::TargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
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

void ABaseAIController::DrawNavigationDebug(const FVector& location, FColor colour)
{
	if (DRONE_AI_DEBUG_ENABLED)
	{
		DrawDebugLine(GetWorld(), mDroneLocation, location, colour, false, 0.5);
		DrawDebugSphere(GetWorld(), location, 100, 10, colour, false, 0.5);
	}
}

void ABaseAIController::ActorSeen(AActor* Actor)
{
	if (!IsTargetValid())
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s's actor seen"), *GetCombatantName());
		FCombatantData data = mCreateCombatantData(Actor);

		if (IsTargetValid(data))
		{
			SetTarget(data);
		}
	}
}

UObjectiveComponent* ABaseAIController::GetClosestUncontrolledObjective()
{
	UObjectiveComponent* closest = nullptr;
	TArray<UObjectiveComponent*> objectives = GetGameMode()->GetObjectives();

	for (UObjectiveComponent* objective : objectives)
	{
		// Check if we don't already control the objective, if we do, then we'll be picking one to defend later
		if (!objective->HasCompleteControl(combatant.GetTeam()))
		{
			if (!IsValid(closest))
			{
				closest = objective;
			}
			else if (FVector::Dist(closest->GetOwner()->GetActorLocation(), GetNavAgentLocation()) > FVector::Dist(objective->GetOwner()->GetActorLocation(), GetNavAgentLocation()))
			{
				closest = objective;
			}
		}
	}

	return  closest;
}

AActor* ABaseAIController::FindEnemyTarget()
{
	TArray<AActor*> actors;

	for (auto combatantFound : GetGameMode()->GetCombatants())
	{
		if (combatantFound->GetTeam() != combatant.GetTeam())
		{
			actors.Add(combatantFound->GetOwner());
		}
	}

	if (actors.Num() > 0)
	{
		return mGetClosestActorInArray(actors, GetOwner()->GetActorLocation());
	}

	return nullptr;
}

void ABaseAIController::LostSightOfActor(AActor* Actor, const FVector& lastSeenLocation)
{
	if (GetTarget() == Actor)
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s's lost sight of  target %s"), *GetCombatantName(), *GetTarget().GetCombatantName());

		if (!GetNextVisibleTarget())
		{
			SetTarget(FCombatantData());
		}
	}
}

void ABaseAIController::OnTargetUnitDied(UCombatantComponent* inKiller)
{
	UE_LOG(LogDroneAI, Log, TEXT("%s's target ( %s) died "), *GetCombatantName(), *GetTarget().GetCombatantName());
	SetTarget(FCombatantData());
}
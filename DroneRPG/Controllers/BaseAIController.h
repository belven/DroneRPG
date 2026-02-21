#pragma once

#include "CoreMinimal.h"
#include "DetourCrowdAIController.h"
#include "DroneRPG/LevelActors/Objective.h"
#include "DroneRPG/Utilities/CombatClasses.h"
#include "DroneRPG/Utilities/Enums.h"
#include "Perception/AIPerceptionTypes.h"
#include "BaseAIController.generated.h"

class UAISenseConfig_Sight;

UCLASS()
class DRONERPG_API ABaseAIController : public ADetourCrowdAIController
{
	GENERATED_BODY()
public:
	ABaseAIController(const FObjectInitializer& ObjectInitializer);
	FString GetStateString(EActionState state);

	virtual void OnPossess(APawn* InPawn) override;
	virtual void BeginPlay() override;
	virtual void SetFiringState(bool firingState);

	virtual	void OwnerAttacked(AActor* attacker);

	bool IsTargetValid() { return IsTargetValid(GetTarget()); };

	virtual bool IsTargetValid(FCombatantData& data);
	bool GetNextVisibleTarget();

	EGameModeType GetCurrentGameMode() const { return currentGameMode; }

	virtual void SetTarget(const FCombatantData& inTarget);
	FCombatantData& GetTarget() { return target; }

	virtual ADroneRPGGameMode* GetGameMode();

	UFUNCTION()
	virtual void TargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void DrawNavigationDebug(const FVector& location, FColor colour);

	FString GetCombatantName() { return combatant.GetCombatantName(); }

	virtual void ActorSeen(AActor* Actor);
	UObjectiveComponent* GetClosestUncontrolledObjective();
	AActor* FindEnemyTarget();
	virtual	void LostSightOfActor(AActor* Actor, const FVector& lastSeenLocation);

	UFUNCTION()
	virtual void OnTargetUnitDied(UCombatantComponent* inKiller);

protected:
	bool isFiring;
	int32 acceptanceRadius;
	EGameModeType currentGameMode;

	UPROPERTY()
	ADroneRPGGameMode* gameMode;

	UPROPERTY()
	UAISenseConfig_Sight* sightConfig;

	UPROPERTY()
	FCombatantData target;

	UPROPERTY()
	FCombatantData combatant;
};
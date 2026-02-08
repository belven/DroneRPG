#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "DroneRPG/Utilities/Enums.h"
#include "Perception/AIPerceptionTypes.h"
#include "DroneBaseAI.generated.h"

class UAISenseConfig_Sight;
class ADroneRPGCharacter;
class AObjective;

UCLASS()
class DRONERPG_API ADroneBaseAI : public AAIController
{
	GENERATED_BODY()
public:
	ADroneBaseAI(const FObjectInitializer& ObjectInitializer);
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnPossess(APawn* InPawn) override;
	void PerformActions();
	void MoveToObjective();
	virtual void BeginPlay() override;

	UFUNCTION()
		void ObjectiveTaken(AObjective* objective);

	UFUNCTION()
		void DroneAttacked(AActor* attacker);

	bool IsTargetValid();

	UFUNCTION()
		void CheckLastLocation();

	EActionState GetCurrentState() const { return currentState; }
	void SetCurrentState(EActionState val) { SetPreviousState(currentState); currentState = val; }

	EActionState GetPreviousState() const { return previousState; }
	void SetPreviousState(EActionState val) { previousState = val; }

	AActor* GetTargetObjective() const { return targetObjective; }
	void SetTargetObjective(AActor* val) { targetObjective = val; }

	EGameModeType GetCurrentGameMode() const { return currentGameMode; }
	void SetCurrentGameMode(EGameModeType val) { currentGameMode = val; }

	UFUNCTION()
	void TargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
private:
//	FTimerHandle TimerHandle_CanCheckForEnemies;
	FTimerHandle TimerHandle_CanPerformActions;
	FTimerHandle TimerHandle_CheckLastLocation;

	float minCaptureDistance;
	float targetRange;
	FRotator lookAt;
	FVector lastLocation;

	bool isFiring;
	bool canCheckForEnemies;
	bool canPerformActions;

	UPROPERTY()
	UAISenseConfig_Sight* sightConfig;

	UPROPERTY()
	AActor* targetObjective;

	UPROPERTY()
	AActor* target;

	EActionState currentState;
	EActionState previousState;
	EGameModeType currentGameMode;

	ADroneRPGCharacter* GetDrone();
	AActor* FindEnemyTarget(float distance = 0);
	void FireShot(const FVector& FireDirection);

	void FindTarget();
	void CalculateObjective();
	void RotateToFace();
	void GetNextVisibleTarget();
	void FindObjective();
	ADroneRPGCharacter* GetDroneTarget();
	void CanPerformActions();
	void AttackTarget(AActor* targetToAttack);

	void DefendingObjective();
	void ReturningToBase();
	bool ShootTargetIfValid();
	void EvadingDamage();
	void AttackingTarget();
	FHitResult LineTraceToLocation(const FVector& startLoc, const FVector& endLocation);
	bool CanSee(AActor* other, const FVector& startLoc);
	FVector GetPredictedLocation(AActor* actor);
	void CapturingObjective();
//	bool GetEnemiesInArea();
};

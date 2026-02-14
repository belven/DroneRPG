#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "DroneRPG/Utilities/CombatClasses.h"
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
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	UFUNCTION()
	void ObjectiveTaken(AObjective* objective);

	UFUNCTION()
	void DroneAttacked(AActor* attacker);

	bool IsTargetValid() { return  IsTargetValid(GetTarget()); };

	bool IsTargetValid(FTargetData& data);

	UFUNCTION()
	void CheckLastLocation();

	EActionState GetCurrentState() const { return currentState; }
	void SetCurrentState(EActionState val);

	EActionState GetPreviousState() const { return previousState; }
	void SetPreviousState(EActionState val);

	AActor* GetTargetObjective() const { return targetObjective; }
	void SetTargetObjective(AActor* val) { targetObjective = val; }

	EGameModeType GetCurrentGameMode() const { return currentGameMode; }
	void SetCurrentGameMode(EGameModeType val);
	FString GetStateString(EActionState state);

	FTargetData& GetTarget()
	{
		return target;
	}

	UFUNCTION()
	void OnUnitDied(UCombatantComponent* inKiller);
	void SetTarget(const FTargetData& inTarget);

	bool CompareState(EActionState state);

	UFUNCTION()
	void TargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
private:
	FTimerHandle TimerHandle_CheckLastLocation;

	float minCaptureDistance;
	float targetRange;
	FRotator lookAt;
	FVector lastLocation;

	bool isFiring;
	bool canCheckForEnemies;

	UPROPERTY()
	UAISenseConfig_Sight* sightConfig;

	UPROPERTY()
	AActor* targetObjective;

	UPROPERTY()
	FTargetData target;

	EActionState currentState;
	EActionState previousState;
	EGameModeType currentGameMode;

	ADroneRPGCharacter* GetDrone();
	AActor* FindEnemyTarget(float distance = 0);
	void FireShot(const FVector& FireDirection);

	void FindTarget();
	void FindSuitableObjective();
	void RotateToFace();
	void GetNextVisibleTarget();
	void FindObjective();
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
};

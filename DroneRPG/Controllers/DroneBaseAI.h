#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "DetourCrowdAIController.h"
#include "DroneRPG/Utilities/CombatClasses.h"
#include "DroneRPG/Utilities/Enums.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "Perception/AIPerceptionTypes.h"
#include "DroneBaseAI.generated.h"

class UAISenseConfig_Sight;
class ADroneRPGCharacter;
class AObjective;

UCLASS()
class DRONERPG_API ADroneBaseAI : public ADetourCrowdAIController
{
	GENERATED_BODY()
public:
	// ReSharper disable once CppNonExplicitConvertingConstructor
	ADroneBaseAI(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaSeconds) override;
	virtual void OnPossess(APawn* InPawn) override;
	void MovingToObjective();
	void PerformActions();
	void RunMoveQuery(const FVector& location, double radius);
	void MoveToObjective();
	virtual void BeginPlay() override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	UFUNCTION()
	void ObjectiveTaken(AObjective* objective);

	UFUNCTION()
	void DroneAttacked(AActor* attacker);

	bool IsTargetValid() { return IsTargetValid(GetTarget()); };

	bool IsTargetValid(FTargetData& data);

	UFUNCTION()
	void CheckLastLocation();

	EActionState GetCurrentState() const { return currentState; }
	void SetCurrentState(EActionState val);

	EActionState GetPreviousState() const { return previousState; }
	void SetPreviousState(EActionState val);

	AObjective* GetTargetObjective() const { return targetObjective; }
	void SetTargetObjective(AObjective* val);

	EGameModeType GetCurrentGameMode() const { return currentGameMode; }
	void SetCurrentGameMode(EGameModeType val);
	FString GetStateString(EActionState state);

	FTargetData& GetTarget() { return target; }

	UFUNCTION()
	void OnUnitDied(UCombatantComponent* inKiller);
	void SetTarget(const FTargetData& inTarget);

	bool CompareState(EActionState state);
	void ActorSeen(AActor* Actor);
	void LostSightOfActor(AActor* Actor, const FVector& lastSeenLocation);

	UFUNCTION()
	void TargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
	void FailedToMoveToLocation(const FVector& location);

	void FindLocationEmptyLocationRequestFinished(TSharedPtr<FEnvQueryResult> Result);
	FVector queryLocation;
	ADroneRPGGameMode* GetGameMode();
private:
	FTimerHandle TimerHandle_CheckLastLocation;

	float minCaptureDistance;
	float targetRange;
	FRotator lookAt;
	FVector lastLocation;
	int32 capsuleSize;

	bool isFiring;
	bool canCheckForEnemies;
	EActionState currentState;
	EActionState previousState;
	EGameModeType currentGameMode;

	/*UPROPERTY()
	UEnvQuery* FindLocationEmptyLocationQuery;*/

	UPROPERTY()
	FEnvQueryRequest FindLocationEmptyLocationRequest;

	UPROPERTY()
	ADroneRPGGameMode* gameMode;

	UPROPERTY()
	UAISenseConfig_Sight* sightConfig;

	UPROPERTY()
	AObjective* targetObjective;

	UPROPERTY()
	FTargetData target;

	UPROPERTY()
	ADroneRPGCharacter* droneCharacter;

	ADroneRPGCharacter* GetDrone();
	AActor* FindEnemyTarget(float distance = 0);
	void FireShot(const FVector& FireDirection);

	void FindTarget();
	void FindSuitableObjective();
	void RotateToFace(float DeltaSeconds);
	bool GetNextVisibleTarget();
	void FindObjective();
	void AttackTarget();

	void DefendingObjective();
	void ReturningToBase();
	bool ShootTargetIfValid();
	void EvadingDamage();
	bool IsNotMoving();
	bool IsTargetInWeaponRange();
	void AttackingTarget();
	FHitResult LineTraceToLocation(const FVector& startLoc, const FVector& endLocation);
	bool CanSee(AActor* other, const FVector& startLoc);
	FVector GetPredictedLocation(AActor* actor);
	void CapturingObjective();
};
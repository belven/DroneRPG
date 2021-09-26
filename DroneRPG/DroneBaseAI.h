#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "DroneRPGGameMode.h"
#include "FunctionLibrary.h"
#include "DroneBaseAI.generated.h"

class ADroneRPGCharacter;
class AObjective;

UCLASS()
class DRONERPG_API ADroneBaseAI : public AAIController
{
	GENERATED_BODY()
public:
	ADroneBaseAI();
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
	void SetCurrentState(EActionState val) { SetPreivousState(currentState); currentState = val; }

	EActionState GetPreivousState() const { return preivousState; }
	void SetPreivousState(EActionState val) { preivousState = val; }

	AActor* GetTargetObjective() const { return targetObjective; }
	void SetTargetObjective(AActor* val) { targetObjective = val; }

	EGameModeType GetCurrentGameMode() const { return currentGameMode; }
	void SetCurrentGameMode(EGameModeType val) { currentGameMode = val; }
private:
	FTimerHandle TimerHandle_CanCheckForEnemies;
	FTimerHandle TimerHandle_CanPerformActions;
	FTimerHandle TimerHandle_CheckLastLocation;

	float minCaptureDistance;
	float targetRange;
	FRotator lookAt;
	FVector lastLocation;

	bool isFiring;
	bool canCheckForEnemies;
	bool canPerformActions;

	AActor* targetObjective;
	AActor* target;

	EActionState currentState;
	EActionState preivousState;
	EGameModeType currentGameMode;

	ADroneRPGCharacter* GetDrone();
	AActor* FindEnemyTarget(float distance = 0);
	void FireShot(FVector FireDirection);

	void FindTarget();
	void CalculateObjective();
	void RotateToFace();
	void FindObjective();
	void ShotTimerExpired();
	void CanCheckForEnemies();
	void CanPerformActions();
	bool AttackTarget(AActor* targetToAttack, bool moveIfCantSee = true);

	void DefendingObjective();
	void ReturningToBase();
	bool ShootAttacker();
	void EvadingDamage();
	void AttackingTarget();
	FHitResult LinetraceToLocation(FVector startLoc, FVector endLocation);
	bool CanSee(AActor* other, FVector startLoc);
	FVector GetPredictedLocation(AActor* actor);
	void CapturingObjective();
	bool GetEnemiesInArea();
};

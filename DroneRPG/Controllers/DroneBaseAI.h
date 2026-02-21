#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "BaseAIController.h"
#include "DroneRPG/Utilities/CombatClasses.h"
#include "DroneRPG/Utilities/Enums.h"
#include "DroneRPG/Weapons/Weapon.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "Perception/AIPerceptionTypes.h"
#include "DroneBaseAI.generated.h"

class UAISenseConfig_Sight;
class ADroneRPGCharacter;
class UObjectiveComponent;

UCLASS()
class DRONERPG_API ADroneBaseAI : public ABaseAIController
{
	GENERATED_BODY()
public:
	// ReSharper disable once CppNonExplicitConvertingConstructor
	ADroneBaseAI(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaSeconds) override;

	void MovingToObjective();
	void PerformActions();
	void MoveToObjective();

	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
	virtual void SetFiringState(bool firingState) override;

	UFUNCTION()
	void ObjectiveTaken(UObjectiveComponent* objective);

	virtual void OwnerAttacked(AActor* attacker) override;

	UFUNCTION()
	void CheckLastLocation();

	EActionState GetCurrentState() const { return currentState; }
	void SetCurrentState(EActionState val);

	EActionState GetPreviousState() const { return previousState; }
	void SetPreviousState(EActionState val);

	UObjectiveComponent* GetTargetObjective() const { return targetObjective; }

	void SetTargetObjective(UObjectiveComponent* val);

	virtual void OnTargetUnitDied(UCombatantComponent* inKiller) override;
	virtual void SetTarget(const FCombatantData& inTarget) override;

	bool CompareState(EActionState state);
	virtual void LostSightOfActor(AActor* Actor, const FVector& lastSeenLocation) override;

	UFUNCTION()
	void FailedToMoveToLocation(const FVector& location);

	void MoveRequestFinished(TSharedPtr<FEnvQueryResult> Result);
	FVector queryLocation;
	bool drawDebug;

	virtual void StopMovement() override;

private:
	FTimerHandle TimerHandle_CheckLastLocation;

	float minCaptureDistance;
	bool isMoving;
	bool isRequestingMovement;
	float targetRange;

	FRotator lookAt;
	FVector lastLocation;

	bool canCheckForEnemies;
	EActionState currentState;
	EActionState previousState;

	UPROPERTY()
	FEnvQueryRequest FindLocationEmptyLocationRequest;

	UPROPERTY()
	FEnvQueryRequest FindEvadeLocationRequest;

	UPROPERTY()
	UObjectiveComponent* targetObjective;

	UPROPERTY()
	ADroneRPGCharacter* droneCharacter;

	ADroneRPGCharacter* GetDrone();

	void FindTarget();
	void FindSuitableObjective();
	void RotateToFace(float DeltaSeconds);
	void FindObjective();

	void DefendingObjective();
	void ReturningToBase();
	float GetWeaponRange();
	void RunMoveQuery(FEnvQueryRequest& query, const FVector& inLocation, float inGetRange, const FString& source);
	void RunMoveQuery(const FVector& location, double radius, const FString& source);
	void EvadingDamage();
	bool IsNotMoving();
	bool IsTargetInWeaponRange();
	bool IsTargetInWeaponRange(const FCombatantData& targetToCheck);
	void AttackingTarget();
	FHitResult LineTraceToLocation(const FVector& startLoc, const FVector& endLocation);
	bool CanSee(AActor* other, const FVector& startLoc);
	FVector GetPredictedLocation(AActor* actor);
	void CapturingObjective();
};
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "DroneBaseAI.generated.h"

class ADroneRPGCharacter;

UENUM(BlueprintType)
enum class  EActionState : uint8 {
	SearchingForObjective,
	MovingToObjective,
	AttackingTarget,
	CapturingObjective,
	EvadingDamage,
	ReturingToBase
};

UENUM(BlueprintType)
enum class  EGameModeType : uint8 {
	Domination,
	TeamDeathMatch,
	Hardpoint,
	AttackDefend,
	Payload
};

UCLASS()
class DRONERPG_API ADroneBaseAI : public AAIController
{
	GENERATED_BODY()
public:
	ADroneBaseAI();

	void CalculateObjective();
	void FindObjective();
	void DroneAttacked(AActor* attacker);
	AActor* FindEnemyTarget(float distance = 0);
	void FindTarget();
	void RotateToFace();
	float minDistance;

	virtual void Tick(float DeltaSeconds) override;

	void AttackingTarget();
	bool IsTargetValid();
	void FireShot(FVector FireDirection);
	bool AttackTarget(AActor* targetToAttack, bool moveIfCantSee = true);
	void MoveToObjective();
	
	TSubclassOf<class ADroneProjectile> projectileClass;

	/** Handle for efficient management of ShotTimerExpired timer */
	FTimerHandle TimerHandle_ShotTimerExpired;

	FRotator lookAt;
	bool bCanFire;
	bool isFiring;
	float FireRate;
	FVector GunOffset;
	class USoundBase* FireSound;

	void CapturingObjective();
	/* Handler for the fire timer expiry */
	void ShotTimerExpired();

	EActionState GetCurrentState() const { return currentState; }
	void SetCurrentState(EActionState val) { SetPreivousState(currentState); currentState = val; }

	EActionState GetPreivousState() const { return preivousState; }
	void SetPreivousState(EActionState val) { preivousState = val; }

	AActor* GetTargetObjective() const { return targetObjective; }
	void SetTargetObjective(AActor* val) { targetObjective = val; }

	EGameModeType GetCurrentGameMode() const { return currentGameMode; }
	void SetCurrentGameMode(EGameModeType val) { currentGameMode = val; }
private:
	AActor* targetObjective;
	AActor* target;
	EActionState currentState;
	EActionState preivousState;
	EGameModeType currentGameMode;
	bool isMovingToObjective;

};

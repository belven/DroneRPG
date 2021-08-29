// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "DroneBaseAI.generated.h"

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
	float minDistance;

	virtual void Tick(float DeltaSeconds) override;

	void FireShot(FVector FireDirection);
	void AttackTarget();
	void MoveToTarget();
	
	TSubclassOf<class ADroneProjectile> projectileClass;

	/** Handle for efficient management of ShotTimerExpired timer */
	FTimerHandle TimerHandle_ShotTimerExpired;

	FRotator lookAt;
	bool bCanFire;
	bool isFiring;
	float FireRate;
	FVector GunOffset;
	class USoundBase* FireSound;

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
	EActionState currentState;
	EActionState preivousState;
	EGameModeType currentGameMode;
	bool isMovingToObjective;

};

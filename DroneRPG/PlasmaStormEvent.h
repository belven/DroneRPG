// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TriggeredEvent.h"
#include "DroneDamagerInterface.h"
#include "PlasmaStormEvent.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class ADroneRPGCharacter;

UCLASS()
class DRONERPG_API APlasmaStormEvent : public ATriggeredEvent, public IDroneDamagerInterface
{
	GENERATED_BODY()

public:
	APlasmaStormEvent();

	virtual void DroneKilled(ADroneRPGCharacter* drone) override;
	virtual FString GetDamagerName() override;

	virtual FString GetEventName() override;
	virtual void TriggerEvent() override;

	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	float GetRadius();
	void SetRadius(float val) { radius = val; }

	float GetDamage() const { return damage; }
	void SetDamage(float val) { damage = val; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plasma Storm")
		float radius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plasma Storm")
		float damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plasma Storm")
		float kills;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plasma Storm")
		float damageDealt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plasma Storm")
		float travelDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plasma Storm")
		float damageRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plasma Storm")
		float moveRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plasma Storm")
		float acceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plasma Storm")
		bool isPlayerHunter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plasma Storm")
		bool isPowerDrainer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plasma Storm")
		float powerDrainLimit;

private:
	void Move();
	UPROPERTY()
		UNiagaraSystem* stormSystem;

	UPROPERTY()
		UNiagaraComponent* stormParticle;

	FTimerHandle TimerHandle_Move;
	FNavLocation targetLocation;
	UStaticMeshComponent* meshComponent;
};

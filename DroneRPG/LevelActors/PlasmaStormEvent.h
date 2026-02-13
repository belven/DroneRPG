#pragma once
#include "CoreMinimal.h"
#include "DroneRPG/TriggeredEvent.h"
#include "PlasmaStormEvent.generated.h"

class UCombatantComponent;
class ADroneRPGGameMode;
class USphereComponent;
class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class DRONERPG_API APlasmaStormEvent : public ATriggeredEvent
{
	GENERATED_BODY()

public:
	APlasmaStormEvent();

	UFUNCTION()
	void UnitKilled(UCombatantComponent* unitKilled);

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

	ADroneRPGGameMode* GetGameMode();

	UPROPERTY()
	UNiagaraComponent* stormParticle;

	UPROPERTY()
	UCombatantComponent* combatantComponent;

	FTimerHandle TimerHandle_Move;
	FNavLocation targetLocation;

	UPROPERTY()
	UStaticMeshComponent* meshComponent;

	UPROPERTY()
	USphereComponent* sphereComponent;

	UPROPERTY()
	ADroneRPGGameMode* gameMode;
};

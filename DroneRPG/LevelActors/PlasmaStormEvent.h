#pragma once
#include "CoreMinimal.h"
#include "DroneRPG/DroneDamagerInterface.h"
#include "DroneRPG/TriggeredEvent.h"
#include "PlasmaStormEvent.generated.h"

class USphereComponent;
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
	virtual EDamagerType GetDamagerType() override;

	virtual FString GetEventName() override;
	virtual void TriggerEvent() override;

	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	float GetRadius();
	void SetRadius(float val) { radius = val; }

	float GetDamage() const { return damage; }
	void SetDamage(float val) { damage = val; }

	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

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

	UPROPERTY()
	UStaticMeshComponent* meshComponent;

	UPROPERTY()
	USphereComponent* sphereComponent;
};

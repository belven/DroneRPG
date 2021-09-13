// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "DroneRPGCharacter.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class ADroneProjectile;
class ARespawnPoint;
class UWeapon;

USTRUCT(BlueprintType)
struct FDroneStats
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		float  health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		float  shields;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		float energy;
};

UCLASS(Blueprintable)
class ADroneRPGCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ADroneRPGCharacter();
	void SetDefaults();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		float energyRegen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		float shieldRegen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		float energyRegenDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		float shieldRegenDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		int32 team;


	UFUNCTION(BlueprintCallable, Category = "Drone")
		FColor GetHealthStatus() const { return healthStatus; }

	UFUNCTION(BlueprintCallable, Category = "Drone")
		void SetHealthStatus(FColor val) { healthStatus = val; }

	bool canRegenShields;
	bool shieldsCritical;
	bool shieldsActive;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* meshComponent;

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	void Respawn();
	void KillDrone();
	void RecieveHit(ADroneProjectile* projectile);

	UFUNCTION(BlueprintCallable, Category = "Drone")
		bool IsAlive();

	UFUNCTION(BlueprintCallable, Category = "Drone")
		bool IsHealthy();

	UFUNCTION(BlueprintCallable, Category = "Drone")
		void FullHeal();

	UFUNCTION()
		void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	bool HasShields();
	ARespawnPoint* GetRespawnPoint();


	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns CursorToWorld subobject **/
	FORCEINLINE class UDecalComponent* GetCursorToWorld() { return CursorToWorld; }

	virtual void BeginPlay() override;


	FTimerHandle TimerHandle_ShieldRegenRestart;
	FTimerHandle TimerHandle_Kill;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
		UNiagaraSystem* auraSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
		UNiagaraSystem* trailSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
		UNiagaraComponent* shieldParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
		UNiagaraComponent* healthParticle;

	int32 GetTeam() const { return team; }
	void SetTeam(int32 val) { team = val; }

	FDroneStats GetCurrentStats() const { return currentStats; }
	void SetCurrentStats(FDroneStats val) { currentStats = val; }
	FDroneStats GetMaxStats() const { return maxStats; }
	void SetMaxStats(FDroneStats val) { maxStats = val; }

	TArray<ADroneRPGCharacter*>& GetDronesInArea() { return dronesInArea; }
	void SetDronesInArea(TArray<ADroneRPGCharacter*> val) { dronesInArea = val; }

	UWeapon* GetWeapon() const { return weapon; }
	void SetWeapon(UWeapon* val) { weapon = val; }
private:
	void CalculateHealthColours();
	void CalculateShieldParticles();
	void CalculateShields(float DeltaSeconds);
	void CalculateEnergy(float DeltaSeconds);
	void StartShieldRegen();
	float ClampValue(float value, float max, float min);

	UPROPERTY()
		UBoxComponent* droneArea;

	UPROPERTY()
		UWeapon* weapon;

	UPROPERTY()
		TArray<ADroneRPGCharacter*> dronesInArea;

	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** A decal that projects to the cursor location. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UDecalComponent* CursorToWorld;


	FDroneStats currentStats;
	FDroneStats maxStats;
	FColor healthStatus;
};


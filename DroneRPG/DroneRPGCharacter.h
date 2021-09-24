// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "DroneRPGCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDroneDied, ADroneRPGCharacter*, drone);

class UNiagaraComponent;
class UNiagaraSystem;
class ADroneProjectile;
class ARespawnPoint;
class UWeapon;
class UMaterialInstanceConstant;

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		int32 team;

	FDroneDied DroneDied;

	ADroneRPGCharacter();
	virtual void BeginDestroy() override;
	void Respawn();
	void KillDrone();
	void DamageDrone(float damage);
	virtual void BeginPlay() override;
	void PulseShield();
	virtual void Tick(float DeltaSeconds) override;
	void RecieveHit(ADroneProjectile* projectile);

	UFUNCTION()
		void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable, Category = "Drone")
		void SetDefaults();

	UFUNCTION(BlueprintCallable, Category = "Drone")
		bool IsAlive();

	UFUNCTION(BlueprintCallable, Category = "Drone")
		bool IsHealthy();

	UFUNCTION(BlueprintCallable, Category = "Drone")
		void FullHeal();

	UFUNCTION(BlueprintCallable, Category = "Drone")
		bool HasShields();

	UFUNCTION(BlueprintCallable, Category = "Drone")
		ARespawnPoint* GetRespawnPoint();

	UFUNCTION(BlueprintCallable, Category = "Drone")
		FColor GetTeamColour();

	UFUNCTION(BlueprintCallable, Category = "Drone")
		FColor GetHealthStatus() const { return healthStatus; }

	UFUNCTION(BlueprintCallable, Category = "Drone")
		void SetHealthStatus(FColor val) { healthStatus = val; }

	UFUNCTION(BlueprintCallable, Category = "Drone")
		int32 GetKills() const { return kills; }
	void SetKills(int32 val) { kills = val; }

	UFUNCTION(BlueprintCallable, Category = "Drone")
		int32 GetDeaths() const { return deaths; }
	void SetDeaths(int32 val) { deaths = val; }

	UFUNCTION(BlueprintCallable, Category = "Drone")
		int32 GetTeam() const { return team; }
	void SetTeam(int32 val) { team = val; }

	UFUNCTION(BlueprintCallable, Category = "Drone")
		FDroneStats GetCurrentStats() const { return currentStats; }
	void SetCurrentStats(FDroneStats val) { currentStats = val; }

	UFUNCTION(BlueprintCallable, Category = "Drone")
		FDroneStats GetMaxStats() const { return maxStats; }
	void SetMaxStats(FDroneStats val) { maxStats = val; }

	UFUNCTION(BlueprintCallable, Category = "Drone")
		UWeapon* GetWeapon() const { return weapon; }
	void SetWeapon(UWeapon* val) { weapon = val; }

	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UDecalComponent* GetCursorToWorld() { return CursorToWorld; }
private:
	bool canRegenShields;
	bool shieldsCritical;
	bool shieldsActive;

	float energyRegen;
	float shieldRegen;
	float energyRegenDelay;
	float shieldRegenDelay;

	float wipeValue;
	float maxWipe;
	float minWipe;

	float smallShieldExp;
	float largeShieldExp;
	float healthParticleSize;

	FDroneStats currentStats;
	FDroneStats maxStats;
	FColor healthStatus;

	int32 kills;
	int32 deaths;
	int32 meshIndex;

	FTimerHandle TimerHandle_ShieldRegenRestart;
	FTimerHandle TimerHandle_Kill;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* meshComponent;

	UPROPERTY(VisibleAnywhere,  BlueprintReadWrite, Category = Mesh, meta = (AllowPrivateAccess = "true"))
		UInstancedStaticMeshComponent* shieldMeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh, meta = (AllowPrivateAccess = "true"))
		UStaticMesh* shieldMesh;

	UPROPERTY()
		UWeapon* weapon;

	UPROPERTY()
		UMaterialInstanceDynamic* shieldDynamicMaterial;

	UPROPERTY()
		UMaterialInstanceConstant* matInstanceConst;

	UPROPERTY()
		TArray<ADroneRPGCharacter*> dronesInArea;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* TopDownCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UDecalComponent* CursorToWorld;

	UPROPERTY()
		UNiagaraSystem* auraSystem;

	UPROPERTY()
		UNiagaraSystem* shieldSystem;

	UPROPERTY()
		UNiagaraComponent* shieldParticle;

	UPROPERTY()
		UNiagaraComponent* healthParticle;

	void CalculateHealthColours();
	void CalculateShieldParticles();

	void SetMaterialColour(FName param, FLinearColor value);
	void SetMaterialFloat(FName param, float value);

	void CalculateShields(float DeltaSeconds);
	void CalculateEnergy(float DeltaSeconds);
	void StartShieldRegen();
	float ClampValue(float value, float max, float min);
};
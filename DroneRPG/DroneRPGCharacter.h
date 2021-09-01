// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DroneRPGCharacter.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class ADroneProjectile;

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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		float energyRegen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		float shieldRegen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		float energyRegenDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		float shieldRegenDelay;

	FDroneStats currentStats;
	FDroneStats maxStats;
	FColor healthStatus;

	bool canRegenShields;
	bool shieldsCritical;
	bool shieldsActive;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* meshComponent;

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	float ClampValue(float value, float max, float min);
	void RecieveHit(ADroneProjectile* projectile);

	void CalculateShieldParticles();

	void CalculateShields(float DeltaSeconds);
	void CalculateEnergy(float DeltaSeconds);

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns CursorToWorld subobject **/
	FORCEINLINE class UDecalComponent* GetCursorToWorld() { return CursorToWorld; }

	virtual void BeginPlay() override;

	void StartShieldRegen();

	/** Handle for efficient management of ShotTimerExpired timer */
	FTimerHandle TimerHandle_ShieldRegenRestart;

	UNiagaraSystem* auraSystem;
	UNiagaraSystem* trailSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Effects)
		UNiagaraComponent* engineParticle;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Effects)
		UNiagaraComponent* shieldParticle;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Effects)
		UNiagaraComponent* healthParticle;

	int32 GetTeam() const { return team; }
	void SetTeam(int32 val) { team = val; }
private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** A decal that projects to the cursor location. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UDecalComponent* CursorToWorld;

	int32 team;
};


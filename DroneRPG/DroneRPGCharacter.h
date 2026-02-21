#pragma once
#include "CoreMinimal.h"
#include "Actors/BaseCharacter.h"
#include "DroneRPGCharacter.generated.h"

class UHealthComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class ADroneProjectile;
class ARespawnPoint;
class UWeapon;
class UMaterialInstanceConstant;
class ADroneRPGGameMode;
class UCombatantComponent;

UCLASS(Blueprintable)
class ADroneRPGCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	void Respawn();

	ADroneRPGCharacter();

	UFUNCTION(BlueprintCallable, Category = "Drone")
	FColor GetTeamColour();

	virtual void UnitDied(UCombatantComponent* killer) override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Drone")
	ARespawnPoint* GetRespawnPoint();

	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UDecalComponent* GetCursorToWorld() { return CursorToWorld; }

private:
	UPROPERTY()
	ARespawnPoint* respawnPoint;

	FTimerHandle TimerHandle_ShieldRegenRestart;
	FTimerHandle TimerHandle_Kill;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* TopDownCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UDecalComponent* CursorToWorld;
};
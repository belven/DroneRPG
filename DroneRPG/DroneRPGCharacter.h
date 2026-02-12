#pragma once
#include "CoreMinimal.h"
#include "Components/CombatantComponent.h"
#include "GameFramework/Character.h"
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
class ADroneRPGCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	void Respawn();

	ADroneRPGCharacter();
	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintCallable, Category = "Drone")
	FColor GetTeamColour();

	UFUNCTION()
	void KillDrone(AActor* killer);
	FString GetDroneName();
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Drone")
	ARespawnPoint* GetRespawnPoint();

	void SetUpDrone();

	UFUNCTION(BlueprintCallable, Category = "Drone")
	int32 GetKills() const { return kills; }
	void SetKills(int32 val) { kills = val; }

	UFUNCTION(BlueprintCallable, Category = "Drone")
	int32 GetDeaths() const { return deaths; }
	void SetDeaths(int32 val) { deaths = val; }

	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION(BlueprintCallable, Category = "Drone")
	int32 GetTeam() const;
	void SetTeam(int32 val);

	UFUNCTION(BlueprintCallable, Category = "Drone")
	UWeapon* GetWeapon() const { return weapon; }
	void SetWeapon(UWeapon* val) { weapon = val; }

	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UDecalComponent* GetCursorToWorld() { return CursorToWorld; }

	UHealthComponent* GetHealthComponent() const { return healthComponent; }
	UCombatantComponent* GetCombatantComponent() const { return combatantComponent; }

	ADroneRPGGameMode* GetGameMode();

private:
	UPROPERTY()
	ARespawnPoint* respawnPoint;
	FString droneName;

	UPROPERTY()
	UHealthComponent* healthComponent;

	UPROPERTY()
	UCombatantComponent* combatantComponent;

	int32 kills;
	int32 deaths;

	FTimerHandle TimerHandle_ShieldRegenRestart;
	FTimerHandle TimerHandle_Kill;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* meshComponent;

	UPROPERTY()
	UWeapon* weapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* TopDownCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UDecalComponent* CursorToWorld;

	UPROPERTY()
	ADroneRPGGameMode* gameMode;
};
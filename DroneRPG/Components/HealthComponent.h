#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class UCombatantComponent;
class UNiagaraSystem;
class UNiagaraComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUnitDied, UCombatantComponent*, killer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FUnitHit, float, damage, UCombatantComponent*, attacker);

USTRUCT(BlueprintType)
struct FUnitStats
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float  health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float  shields;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float energy;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DRONERPG_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();
	void DamageShields(float& damage);

	void ReceiveDamage(float damage, UCombatantComponent* damager);

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
	FColor GetHealthStatus() const { return healthStatus; }

	UFUNCTION(BlueprintCallable, Category = "Drone")
	void SetHealthStatus(FColor val) { healthStatus = val; }

	UFUNCTION(BlueprintCallable, Category = "Drone")
	FUnitStats GetCurrentStats() const { return currentStats; }
	void SetCurrentStats(FUnitStats val) { currentStats = val; }

	UFUNCTION(BlueprintCallable, Category = "Drone")
	FUnitStats GetMaxStats() const { return maxStats; }
	void SetMaxStats(FUnitStats val) { maxStats = val; }

	FColor GetTeamColour() {
		return teamColour;
	}

	void SetTeamColour(FColor colour);
	FUnitDied OnUnitDied;

	FUnitHit OnUnitHit;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	bool canRegenShields;
	bool shieldsCritical;
	bool shieldsActive;

	float energyRegen;
	float shieldRegen;
	float energyRegenDelay;
	float shieldRegenDelay;
	FColor teamColour;

	float wipeValue;
	float maxWipe;
	float minWipe;

	float smallShieldExp;
	float largeShieldExp;
	float healthParticleSize;

	FUnitStats currentStats;
	FUnitStats maxStats;
	FColor healthStatus;

	FTimerHandle TimerHandle_ShieldRegenRestart;
	int32 meshIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	UInstancedStaticMeshComponent* shieldMeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	UStaticMesh* shieldMesh;

	UPROPERTY()
	UMaterialInstanceConstant* matInstanceConst;
	UPROPERTY()
	UMaterialInstanceDynamic* shieldDynamicMaterial;

	UPROPERTY()
	UNiagaraSystem* auraSystem;

	UPROPERTY()
	UNiagaraComponent* healthParticle;

	void CalculateHealthColours();
	void CalculateShieldParticles();

	void SetMaterialColour(FName param, FLinearColor value);
	void SetMaterialFloat(FName param, float value);

	void CalculateShields(float DeltaSeconds);
	void CalculateEnergy(float DeltaSeconds);
	void StartShieldRegen();
	void PulseShield();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;		
};
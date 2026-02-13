#pragma once
#include "DroneProjectile.generated.h"

class UHealthComponent;
class UCombatantComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;
class UNiagaraComponent;
class UNiagaraSystem;

USTRUCT(BlueprintType)
struct FTargetData
{
	GENERATED_USTRUCT_BODY()

	FTargetData() : combatantComponent(nullptr), healthComponent(nullptr)
	{
	}

	FTargetData(UCombatantComponent* inCombatantComponent, UHealthComponent* inHealthComponent)
		: isSet(true),
		combatantComponent(inCombatantComponent), healthComponent(inHealthComponent)
	{
	}

	bool isSet = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	UCombatantComponent* combatantComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	UHealthComponent* healthComponent;
};

UCLASS()
class DRONERPG_API ADroneProjectile : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	UCombatantComponent* shooter;
	float damage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY()
	UNiagaraSystem* trailSystem;

	UPROPERTY()
	UNiagaraComponent* trialParticle;

	void SetUpCollision();
	void IgnoreActor(AActor* actor);

	virtual void SetTarget(FTargetData targetData);
	FTargetData CreateTargetData(AActor* actor);

	UPROPERTY()
	USoundBase* FireSound;

	UPROPERTY()
	USoundBase* HitSound;

public:
	ADroneProjectile();

	static const float Default_Initial_Speed;
	static const float Default_Initial_Lifespan;

	UPROPERTY()
	FTargetData target;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	FORCEINLINE UStaticMeshComponent* GetProjectileMesh() const { return ProjectileMesh; }
	FORCEINLINE UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	UCombatantComponent* GetShooter();
	void SetShooter(UCombatantComponent* val);

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	float GetDamage() const { return damage; }
	void SetDamage(float val) { damage = val; }
};
#pragma once
#include "DroneRPG/Utilities/CombatClasses.h"
#include "DroneProjectile.generated.h"

class UHealthComponent;
class UCombatantComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;
class UNiagaraComponent;
class UNiagaraSystem;

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
	virtual	void SetShooter(UCombatantComponent* val);

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	float GetDamage() const { return damage; }
	void SetDamage(float val) { damage = val; }
};
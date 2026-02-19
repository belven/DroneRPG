#pragma once
#include "DroneRPG/Utilities/CombatClasses.h"
#include "DroneProjectile.generated.h"

class USphereComponent;
class UHealthComponent;
class UCombatantComponent;
class UProjectileMovementComponent;
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

	UFUNCTION()
	virtual void OnBaseProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY()
	UNiagaraSystem* trailSystem;

	UPROPERTY()
	UNiagaraComponent* trialParticle;

	UPROPERTY()
	USphereComponent* CollisionComp;

	virtual void SetTarget(FCombatantData targetData);
	virtual	void HItValidTarget(const FCombatantData& targetData);
	bool CheckIfValidTarget(const FCombatantData& targetData);
	void ActorDetected(AActor* OtherActor);

	UPROPERTY()
	USoundBase* FireSound;

	UPROPERTY()
	USoundBase* HitSound;

public:
	ADroneProjectile();

	static const float Default_Initial_Speed;
	static const float Default_Initial_Lifespan;

	UPROPERTY()
	FCombatantData target;

	FORCEINLINE UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	UCombatantComponent* GetShooter();
	virtual	void SetShooter(UCombatantComponent* val);

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	float GetDamage() const { return damage; }
	void SetDamage(float val) { damage = val; }
};
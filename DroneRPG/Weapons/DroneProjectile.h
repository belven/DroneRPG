#pragma once
#include "../DroneDamagerInterface.h"
#include "DroneProjectile.generated.h"

class UProjectileMovementComponent;
class UStaticMeshComponent;
class ADroneRPGCharacter;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class DRONERPG_API ADroneProjectile : public AActor, public IDroneDamagerInterface
{
	GENERATED_BODY()

protected:
	virtual void DroneKilled(ADroneRPGCharacter* drone) override;
	virtual FString GetDamagerName() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY()
	ADroneRPGCharacter* shooter;
	float damage;

	/** Sphere collision component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Projectile, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* ProjectileMesh;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY()
		UNiagaraSystem* trailSystem;

	UPROPERTY()
		UNiagaraComponent* trialParticle;

	void SetUpCollision();
	void IgnoreActor(AActor* actor);
	virtual int32 GetDamagerTeam() override;

	UPROPERTY()
	ADroneRPGCharacter* target;
	UPROPERTY()
	USoundBase* FireSound;
	UPROPERTY()
	USoundBase* HitSound;

public:
	ADroneProjectile();

	static const float Default_Initial_Speed;
	static const float Default_Initial_Lifespan;

	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
		virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	FORCEINLINE UStaticMeshComponent* GetProjectileMesh() const { return ProjectileMesh; }
	FORCEINLINE UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

	UFUNCTION(BlueprintCallable, Category = "Projectile")
		ADroneRPGCharacter* GetShooter();
	void SetShooter(ADroneRPGCharacter* val);

	UFUNCTION(BlueprintCallable, Category = "Projectile")
		float GetDamage() const { return damage; }
	void SetDamage(float val) { damage = val; }

	ADroneRPGCharacter* GetTarget() const { return target; }
	virtual void SetTarget(ADroneRPGCharacter* val);
};

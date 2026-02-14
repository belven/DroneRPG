#pragma once
#include "CoreMinimal.h"
#include "DroneProjectile.h"
#include "Rocket.generated.h"

class UHealthComponent;
class USphereComponent;

UCLASS()
class DRONERPG_API ARocket : public ADroneProjectile
{
	GENERATED_BODY()
public:
	ARocket();

	bool CheckIfValidTarget(const FTargetData& targetData);
	bool SetTargetIfValid(const FTargetData& targetData);

	static const float Default_Initial_Speed;
	static const float Default_Initial_Lifespan;

	virtual  void SetTarget(FTargetData targetData) override;
	void DealDamage();
	virtual	void SetShooter(UCombatantComponent* val) override;

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
private:
	UPROPERTY()
	USphereComponent* sphereComponent;
};
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

	bool SetTargetIfValid(const FCombatantData& targetData);

	static const float Default_Initial_Speed;
	static const float Default_Initial_Lifespan;

	UFUNCTION()
	void TargetDied(UCombatantComponent* inKiller);
	virtual  void SetTarget(FCombatantData targetData) override;
	void DealDamage();
	void TargetOverlappingActors();
	virtual	void SetShooter(UCombatantComponent* val) override;

	virtual	void HItValidTarget(const FCombatantData& targetData) override;

	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
private:
	UPROPERTY()
	USphereComponent* sphereComponent;
};
#pragma once
#include "CoreMinimal.h"
#include "DroneProjectile.h"
#include "Rocket.generated.h"

class ADroneRPGCharacter;

UCLASS()
class DRONERPG_API ARocket : public ADroneProjectile
{
	GENERATED_BODY()
public:
	ARocket();
	static const float Default_Initial_Speed;
	static const float Default_Initial_Lifespan;

	virtual void SetTarget(ADroneRPGCharacter* val) override;
	virtual void Tick(float DeltaTime) override;

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
private:
	bool canCheckForEnemies;
	FTimerHandle TimerHandle_CanCheckForEnemies;
	void CanCheckForEnemies();
};
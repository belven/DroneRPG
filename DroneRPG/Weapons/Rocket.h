#pragma once
#include "CoreMinimal.h"
#include "DroneProjectile.h"
#include "Rocket.generated.h"

class USphereComponent;
class ADroneRPGCharacter;

UCLASS()
class DRONERPG_API ARocket : public ADroneProjectile
{
	GENERATED_BODY()
public:
	ARocket();
	bool CheckIfValidTarget(ADroneRPGCharacter* droneFound);
	static const float Default_Initial_Speed;
	static const float Default_Initial_Lifespan;

	void SetTeam(int32 inTeam);

	virtual void SetTarget(ADroneRPGCharacter* val) override;

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	int32 team;
private:
	UPROPERTY()
	USphereComponent* sphereComponent;
};
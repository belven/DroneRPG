#pragma once
#include "CoreMinimal.h"
#include "KeyActor.h"
#include "RespawnPoint.generated.h"

class USphereComponent;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class DRONERPG_API ARespawnPoint : public AKeyActor
{
	GENERATED_BODY()

public:
	ARespawnPoint();
	FVector GetSpawnLocation();
	void SpawnTeam();

	virtual void BeginPlay() override;

	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	int32 GetTeamSize() const { return teamSize; }
	void SetTeamSize(int32 inTeamSize) { teamSize = inTeamSize; }
	int32 GetTeam() const { return team; }
	void SetTeam(int32 val);
private:
	void SetupParticles();
	int32 team;

	int32 teamSize;

	UPROPERTY()
	UNiagaraSystem* auraSystem;

	UPROPERTY()
	UNiagaraComponent* captureParticle;

	UPROPERTY()
	USphereComponent* respawnArea;
};

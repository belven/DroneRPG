#pragma once
#include "CoreMinimal.h"
#include "KeyActor.h"
#include "RespawnPoint.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class ADroneRPGCharacter;
class UBoxComponent;

UCLASS()
class DRONERPG_API ARespawnPoint : public AKeyActor
{
	GENERATED_BODY()

public:
	ARespawnPoint();

protected:
	void SpawnTeam();
	virtual void BeginPlay() override;

	void RespawnCharacter(ADroneRPGCharacter* character);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		int32 team;

		int32 teamSize;
	UPROPERTY()
		UBoxComponent* respawnArea;
public:

	UFUNCTION()
		void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void Tick(float DeltaTime) override;

	int32 GetTeam() const { return team; }
	void SetTeam(int32 val) { team = val; }

	UPROPERTY()
	UNiagaraSystem* auraSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Effects)
	UNiagaraComponent* captureParticle;
};

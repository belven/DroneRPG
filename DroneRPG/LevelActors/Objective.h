#pragma once
#include "CoreMinimal.h"
#include "KeyActor.h"
#include <NiagaraComponent.h>
#include "Objective.generated.h"

class ADroneRPGGameMode;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectiveClaimed, AObjective*, objective);

class UBoxComponent;
class ADroneRPGCharacter;
class UNiagaraSystem;

UCLASS()
class DRONERPG_API AObjective : public AKeyActor
{
	GENERATED_BODY()

public:
	AObjective();

	UFUNCTION()
		void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	void CheckForOverlaps();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
		void UnitDied(AActor* unit);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Effects)
		UNiagaraComponent* captureParticle;

	int32 GetAreaOwner() const { return areaOwner; }
	void SetAreaOwner(int32 val);

	bool HasCompleteControl(int32 team);

	FObjectiveClaimed OnObjectiveClaimed;

	FName GetObjectiveName() const { return objectiveName; }
	void SetObjectiveName(FName val) { objectiveName = val; }

	FColor GetCurrentColour() const { return currentColour; }
	void SetCurrentColour(FColor val) { currentColour = val; }

	float GetCurrentControl() const { return currentControl; }
	int32 GetMaxControl() const { return maxControl; }
	float GetCurrentControlPercent();
	TArray<ADroneRPGCharacter*> GetDronesInArea() const { return dronesInArea; }
protected:
	virtual void BeginPlay() override;

	void CalculateOwnership();
	void UpdateColour();
	void CalculateClaim();

	void Add(ADroneRPGCharacter* drone);
	void Remove(ADroneRPGCharacter* drone);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
		int32 areaOwner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
		int32 previousAreaOwner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
		float currentControl;

		ADroneRPGGameMode* GetGameMode();
private:
	UPROPERTY()
	ADroneRPGGameMode* gameMode;

	UPROPERTY()
		UBoxComponent* objectiveArea;

	UPROPERTY()
		TArray<ADroneRPGCharacter*> dronesInArea;

	int32 minControl;
	int32 maxControl;
	int32 smallParticle;
	int32 bigParticle;
	float overlapTimePassed;
	float overlapTimeRate;

	UPROPERTY()
		TArray<int32> teamsInArea;

	UPROPERTY()
		FColor currentColour;

	UPROPERTY()
		bool fullClaim;

	UPROPERTY()
		FName objectiveName;

	UPROPERTY()
		UNiagaraSystem* auraSystem;
};

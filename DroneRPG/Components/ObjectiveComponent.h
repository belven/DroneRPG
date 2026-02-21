#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DroneRPG/LevelActors/Objective.h"
#include "ObjectiveComponent.generated.h"

class UNiagaraComponent;
class USphereComponent;
class UBoxComponent;
class UCombatantComponent;
class UNiagaraSystem;
class ADroneRPGGameMode;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectiveClaimed, UObjectiveComponent*, objective);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DRONERPG_API UObjectiveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UObjectiveComponent();
	float GetSize() { return size; }
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	void CheckForOverlaps();

	UFUNCTION()
	void UnitDied(UCombatantComponent* unit);

	int32 GetAreaOwner() const { return areaOwner; }
	void SetAreaOwner(int32 val);

	bool HasCompleteControl(int32 team);
	void SetSize(float inKeyActorSize) { size = inKeyActorSize; }

	FObjectiveClaimed OnObjectiveClaimed;

	FName GetObjectiveName() const { return objectiveName; }
	void SetObjectiveName(FName val) { objectiveName = val; }

	FColor GetCurrentColour() const { return currentColour; }
	void SetCurrentColour(FColor val) { currentColour = val; }

	float GetCurrentControl() const { return currentControl; }
	int32 GetMaxControl() const { return maxControl; }
	float GetCurrentControlPercent();
	TArray<UCombatantComponent*> GetDronesInArea() const { return combatantsInArea; }

protected:
	void SetAngle(UNiagaraComponent* comp, float angle);

	void CalculateOwnership();
	void UpdateColour();
	void CalculateClaim();

	void Add(UCombatantComponent* combatant);
	void Remove(UCombatantComponent* combatant);

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
	USphereComponent* objectiveArea;

	UPROPERTY()
	TArray<UCombatantComponent*> combatantsInArea;

	int32 minControl;
	int32 maxControl;
	int32 smallParticle;
	int32 bigParticle;
	float overlapTimePassed;
	float overlapTimeRate;
	float size;

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

	UPROPERTY()
	UNiagaraComponent* currentTeamParticles;

	UPROPERTY()
	UNiagaraComponent* transitioningParticles;
};
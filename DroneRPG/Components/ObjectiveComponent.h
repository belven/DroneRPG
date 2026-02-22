#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DroneRPG/Utilities/CombatClasses.h"
#include "ObjectiveComponent.generated.h"

class UNiagaraComponent;
class USphereComponent;
class UNiagaraSystem;
class ADroneRPGGameMode;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectiveClaimed, UObjectiveComponent*, objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FObjectiveParticlesSetup);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DRONERPG_API UObjectiveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UObjectiveComponent();

	FObjectiveClaimed OnObjectiveClaimed;
	FObjectiveParticlesSetup OnObjectiveParticlesSetup;
	bool setupComplete;
	static FColor UNCLAIMED_COLOUR;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UNiagaraComponent* SpawnSystemAttached(FName name);
	void SetupParticles(UNiagaraComponent** inNiagaraComponent, const FString& inStr, int32 angle);

	int32 GetCurrentOwningTeam();
	void UpdateColour();
	float GetCurrentControlPercent();

	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	void CheckForOverlaps();

	UFUNCTION()
	void UnitDied(AActor* unitKilled, UCombatantComponent* killer);

	float GetSize() { return size; }
	void SetTeamClaim(int32 team);

	int32 GetAreaOwner() const { return areaOwner; }
	void SetAreaOwner(int32 val);

	bool HasCompleteControl(int32 team);
	void SetSize(float inKeyActorSize);

	int32 GetPreviousAreaOwner() const { return previousAreaOwner; }
	void SetPreviousAreaOwner(int32 inPreviousAreaOwner) { previousAreaOwner = inPreviousAreaOwner; }

	UNiagaraComponent* GetCurrentTeamParticles() { return currentTeamParticles; }
	UNiagaraComponent* GetTransitioningParticles() { return transitioningParticles; }

	void SetAngle(UNiagaraComponent* comp, float angle);

	FString GetObjectiveName();
	void SetObjectiveName(FString val) { objectiveName = val; }

	FColor GetCurrentColour() const { return currentColour; }
	void SetCurrentColour(FColor val) { currentColour = val; }

	float GetCurrentControl() const { return currentControl; }
	int32 GetMaxControl() const { return maxControl; }

	void SetCurrentControl(float inCurrentControl) { currentControl = inCurrentControl; }

	bool IsFullClaim() const { return fullClaim; }
	void SetFullClaim(bool inFullClaim) { fullClaim = inFullClaim; }

	TArray<FCombatantData> GetCombatantsInArea() const { return combatantsInArea; }

	int32 GetScoreMultiplier() const { return scoreMultiplier; }
	void SetScoreMultiplier(int32 inScoreMultiplier) { scoreMultiplier = inScoreMultiplier; }

	USphereComponent* GetObjectiveArea() const { return objectiveArea; }
	void SetObjectiveArea(USphereComponent* inObjectiveArea) { objectiveArea = inObjectiveArea; }

protected:
	void CalculateOwnership();
	void CalculateClaim();

	void Add(FCombatantData combatant);
	void Remove(const FCombatantData& combatant);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	int32 areaOwner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	int32 previousAreaOwner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	float currentControl;

	ADroneRPGGameMode* GetGameMode();
private:
	static FName RADIUS;
	static FName COLOUR;
	static FName SIZE;
	static FName PERCENT;
	static FName ROTATION;

	UPROPERTY()
	ADroneRPGGameMode* gameMode;

	UPROPERTY()
	USphereComponent* objectiveArea;

	UPROPERTY()
	TArray<FCombatantData> combatantsInArea;

	int32 minControl;
	int32 maxControl;
	int32 smallParticle;
	int32 bigParticle;
	float overlapTimePassed;
	float overlapTimeRate;
	float size;
	int32 scoreMultiplier;

	UPROPERTY()
	TArray<int32> teamsInArea;

	UPROPERTY()
	FColor currentColour;

	UPROPERTY()
	bool fullClaim;

	UPROPERTY()
	FString objectiveName;

	UPROPERTY()
	UNiagaraSystem* auraSystem;

	UPROPERTY()
	UNiagaraComponent* currentTeamParticles;

	UPROPERTY()
	UNiagaraComponent* transitioningParticles;
};
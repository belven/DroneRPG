#pragma once
#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "Turret.generated.h"

class UObjectiveComponent;

UCLASS()
class DRONERPG_API ATurret : public ABaseCharacter
{
	GENERATED_BODY()
public:
	UFUNCTION()
	void ObjectiveClaimed(UObjectiveComponent* inObjective);

	virtual void UnitDied(AActor* unitKilled, UCombatantComponent* inKiller) override;

	UFUNCTION()
	void ObjectiveParticlesSetup();
	ATurret();
	virtual void SetTeam(int32 newTeam) override;
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;

private:
	UPROPERTY()
	UObjectiveComponent* objectiveComponent;
};

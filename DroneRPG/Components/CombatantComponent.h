#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include <DroneRPG/Utilities/Enums.h>
#include "CombatantComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUnitKilled, AActor*, unitKilled);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DRONERPG_API UCombatantComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatantComponent();
	FString GetCombatantName() { return name; };
	EDamagerType GetCombatantType() { return type; };
	int32 GetTeam() { return team; };

	void SetupCombatantComponent(FString inName, EDamagerType inType, int32 inTeam);

	UFUNCTION()
	void UnitKilled(AActor* unitKilled);

	FUnitKilled OnUnitKilled;
private:
	int32 team;
	FString name;
	EDamagerType type;
};

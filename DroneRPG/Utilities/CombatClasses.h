#pragma once
#include "CoreMinimal.h"
#include "DroneRPG/Components/CombatantComponent.h"
#include "DroneRPG/Components/HealthComponent.h"
#include "CombatClasses.generated.h"

#define mGetComponent(actor, clazz) Cast<clazz>(actor->GetComponentByClass(clazz::StaticClass()))
#define mGetCombatantComponent(actor) mGetComponent(actor, UCombatantComponent)
#define mGetHealthComponent(actor) mGetComponent(actor, UHealthComponent)
#define mCreateCombatantData UCombatClasses::CreateCombatantData

USTRUCT(BlueprintType)
struct FCombatantData
{
	GENERATED_USTRUCT_BODY()

	FCombatantData() : combatantComponent(nullptr), healthComponent(nullptr)
	{
	}

	FCombatantData(UCombatantComponent* inCombatantComponent, UHealthComponent* inHealthComponent)
		: isSet(true),
		combatantComponent(inCombatantComponent), healthComponent(inHealthComponent)
	{
	}

	bool isSet = false;

	bool IsValid() const
	{
		return combatantComponent != NULL && healthComponent != NULL;
	}

	bool IsAlive() const
	{
		return healthComponent != NULL ? healthComponent->IsAlive() : false;
	}

	FString GetCombatantName()
	{
		return combatantComponent != NULL ? combatantComponent->GetCombatantName() : "Unknown";
	}

	int32 GetTeam() const
	{
		return combatantComponent != NULL ? combatantComponent->GetTeam() : -10;
	}

	// ReSharper disable once CppNonExplicitConversionOperator
	operator AActor* ()
	{
		return combatantComponent != NULL ? combatantComponent->GetOwner() : NULL;
	}

	FVector GetActorLocation() const
	{
		return combatantComponent != NULL ? combatantComponent->GetOwner()->GetActorLocation() : healthComponent != NULL ? healthComponent->GetOwner()->GetActorLocation() : FVector::ZeroVector;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	UCombatantComponent* combatantComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	UHealthComponent* healthComponent;
};

UCLASS()
class DRONERPG_API UCombatClasses : public UObject
{
	GENERATED_BODY()
public:
	static FCombatantData CreateCombatantData(AActor* actor);
};

inline FCombatantData UCombatClasses::CreateCombatantData(AActor* actor)
{
	if (IsValid(actor))
	{
		return FCombatantData(mGetCombatantComponent(actor), mGetHealthComponent(actor));
	}
	return FCombatantData();
}
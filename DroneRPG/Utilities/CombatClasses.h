#pragma once
#include "CoreMinimal.h"
#include "DroneRPG/Components/CombatantComponent.h"
#include "DroneRPG/Components/HealthComponent.h"
#include "CombatClasses.generated.h"

#define mGetComponent(actor, clazz) Cast<clazz>(actor->GetComponentByClass(clazz::StaticClass()))
#define mGetCombatantComponent(actor) mGetComponent(actor, UCombatantComponent)
#define mGetHealthComponent(actor) mGetComponent(actor, UHealthComponent)
#define mCreateTargetData UCombatClasses::CreateTargetData

USTRUCT(BlueprintType)
struct FTargetData
{
	GENERATED_USTRUCT_BODY()

	FTargetData() : combatantComponent(nullptr), healthComponent(nullptr)
	{
	}

	FTargetData(UCombatantComponent* inCombatantComponent, UHealthComponent* inHealthComponent)
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
	static FTargetData CreateTargetData(AActor* actor);
};

inline FTargetData UCombatClasses::CreateTargetData(AActor* actor)
{
	if (IsValid(actor))
	{
		return FTargetData(mGetCombatantComponent(actor), mGetHealthComponent(actor));
	}
	return FTargetData();
}
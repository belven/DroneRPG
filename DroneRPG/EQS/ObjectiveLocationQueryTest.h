#pragma once
#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "ObjectiveLocationQueryTest.generated.h"

UCLASS()
class DRONERPG_API UObjectiveLocationQueryTest : public UEnvQueryTest
{
	GENERATED_BODY()
public:

	UObjectiveLocationQueryTest(const FObjectInitializer& ObjectInitializer);
	bool CanNavigateToPoint(ACharacter* character, const FVector& actorLocation, const FVector& ItemLocation) const;
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	bool CanSeePoint(const FVector& contextLocation, const FVector& ItemLocation) const;

	UPROPERTY(EditDefaultsOnly, Category = Trace)
	TSubclassOf<UEnvQueryContext> Context;

	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;
};

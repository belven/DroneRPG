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
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

	UPROPERTY(EditDefaultsOnly, Category = Trace)
	TSubclassOf<UEnvQueryContext> Context;

	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;
};

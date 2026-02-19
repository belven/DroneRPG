#pragma once
#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EvadingLocationQueryTest.generated.h"

UCLASS()
class DRONERPG_API UEvadingLocationQueryTest : public UEnvQueryTest
{
	GENERATED_BODY()
public:

	UEvadingLocationQueryTest(const FObjectInitializer& ObjectInitializer);
	EDrawDebugTrace::Type GetDrawDebug() const;
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	bool CanSeePoint(const FVector& contextLocation, const FVector& ItemLocation) const;

	UPROPERTY(EditDefaultsOnly, Category = Trace)
	TSubclassOf<UEnvQueryContext> Context;

	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;
	
};

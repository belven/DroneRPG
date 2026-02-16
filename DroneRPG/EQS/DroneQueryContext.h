#pragma once
#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "DroneQueryContext.generated.h"

UCLASS()
class DRONERPG_API UDroneQueryContext : public UEnvQueryContext
{
	GENERATED_BODY()
public:
	static UDroneQueryContext* CreateDroneQueryContext(FVector inLocation);

	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;

protected:
	FVector location;
};

#include "DroneQueryContext.h"
#include <EnvironmentQuery/Items/EnvQueryItemType_Point.h>
#include <DroneRPG/Controllers/DroneBaseAI.h>

UDroneQueryContext* UDroneQueryContext::CreateDroneQueryContext(FVector inLocation)
{
	UDroneQueryContext* context = NewObject<UDroneQueryContext>();
	context->location = inLocation;
	return context;
}

void UDroneQueryContext::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);

	ADroneBaseAI* ai = Cast<ADroneBaseAI>(QueryInstance.Owner.Get());

	if (IsValid(ai)) 
	{
		UEnvQueryItemType_Point::SetContextHelper(ContextData, ai->queryLocation);
	}
}

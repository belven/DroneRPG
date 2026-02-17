#include "DroneQueryContext.h"
#include <EnvironmentQuery/Items/EnvQueryItemType_Point.h>
#include <DroneRPG/Controllers/DroneBaseAI.h>

void UDroneQueryContext::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);

	ADroneBaseAI* ai = Cast<ADroneBaseAI>(QueryInstance.Owner.Get());

	if (IsValid(ai)) 
	{
		UEnvQueryItemType_Point::SetContextHelper(ContextData, ai->queryLocation);
	}
}

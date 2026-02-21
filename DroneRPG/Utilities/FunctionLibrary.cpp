#include "FunctionLibrary.h"

#include "Components/ShapeComponent.h"

FString UFunctionLibrary::GetColourString(FColor color)
{
	return color.ToString();
}

FVector UFunctionLibrary::GetClosestLocation(const FVector& locationA, const FVector& locationB, const FVector& origin)
{
	double distA = FVector::Dist(locationA, origin);
	double distB = FVector::Dist(locationB, origin);

	return distA < distB ? locationA : locationB;
}

void UFunctionLibrary::SetupOverlap(UShapeComponent* comp)
{
	comp->SetCollisionProfileName(TEXT("Trigger"));
	comp->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	comp->SetGenerateOverlapEvents(true);
}

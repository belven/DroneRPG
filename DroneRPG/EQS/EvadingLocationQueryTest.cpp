#include "EvadingLocationQueryTest.h"

#include "NavigationSystem.h"
#include "DroneRPG/DroneRPG.h"
#include "DroneRPG/DroneRPGCharacter.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "Kismet/KismetSystemLibrary.h"

#define mSphereTraceMultiEQS(start, end, radius, hits) UKismetSystemLibrary::SphereTraceMulti(GetWorld(), start, end, radius, ETraceTypeQuery::TraceTypeQuery1, true, ignore, EDrawDebugTrace::None, hits, true);

UEvadingLocationQueryTest::UEvadingLocationQueryTest(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{	// Set our ItemType to a vector, as the items (grid points) we're looking for, are locations in the world
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();

	// Set Filter Type to range, this will then be a curve of values which allows us to have scores from 0 to 1. I need to look into this more to understand it better
	FilterType = EEnvTestFilterType::Match;

	// If we're filtering only, then we can only pass or fail locations, and scoring only just manipulates the current scores of items.
	// We want to do both, as we need to filter out of range and out of line of sight items but also score in range and in line of sight items higher, the closer they are to our current location
	TestPurpose = EEnvTestPurpose::Filter;

	// Scores that are higher considered better
	ScoringEquation = EEnvTestScoreEquation::Constant;
}

EDrawDebugTrace::Type UEvadingLocationQueryTest::GetDrawDebug() const
{
	return DRONE_EQS_DEBUG_ENABLED ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
}

bool UEvadingLocationQueryTest::CanSeePoint(const FVector& contextLocation, const FVector& ItemLocation) const
{
	TArray<AActor*> ignore;
	bool canSee = true;
	TArray<FHitResult> hits;
	FVector endLoction = ItemLocation;

	// Create a sphere trace, slightly larger than the characters capsule, so we make sure there's enough room to shoot
	UKismetSystemLibrary::SphereTraceMulti(GetWorld(), ItemLocation, contextLocation, 20, TraceTypeQuery1, true, ignore, GetDrawDebug(), hits, true);

	for (FHitResult hit : hits)
	{
		AActor* actor = hit.GetActor();

		if (IsValid(actor)) 
		{
			// Did we hit something?
			if (hit.bBlockingHit && !actor->Tags.Contains("Floor"))
			{
				// If we hit something that's not are target FIRST, then there's something else in the way, and we should invalidate that location
				if (actor->GetComponentsCollisionResponseToChannel(ECC_Pawn) == ECR_Block && !actor->StaticClass()->IsChildOf(ADroneRPGCharacter::StaticClass()))
				{
					canSee = false;
					endLoction = hit.Location;
					break;
				}
			}
		}
	}

	if (DRONE_DEBUG_ENABLED) 
	{
		FColor color = canSee ? FColor::Yellow : FColor::Red;
		DrawDebugLine(GetWorld(), contextLocation, endLoction, color, false, 0.5);
		DrawDebugSphere(GetWorld(), endLoction, 300, 10, color, false, 0.5);
	}
	return canSee;
}

void UEvadingLocationQueryTest::RunTest(FEnvQueryInstance& QueryInstance) const
{
	TArray<FVector> ContextLocations;

	// This will get the locations created by our context, in our case it's a single location based on the AI Controllers GetLastKnowLocation
	if (!QueryInstance.PrepareContext(this->Context, ContextLocations))
	{
		return;
	}

	FVector contextLocation = ContextLocations[0];

	if (contextLocation != FVector::ZeroVector) 
	{
		UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
		for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
		{
			// Get the current item as a FVector
			FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
			FNavLocation Projected;
			NavSys->ProjectPointToNavigation(ItemLocation, Projected, FVector(1000, 1000, 1000), nullptr, nullptr);
			ItemLocation = Projected.Location;

			// Do we have clear Line of sight to the location?
			if (CanSeePoint(contextLocation, ItemLocation))
			{
				// Set the location as passed
				It.ForceItemState(EEnvItemStatus::Passed, 1);
			}
			// We don't have line of Sight to the target
			else
			{
				It.ForceItemState(EEnvItemStatus::Failed);
			}
		}
	}
}

FText UEvadingLocationQueryTest::GetDescriptionTitle() const
{
	return FText::FromString(FString::Printf(TEXT("Evading damage location Test")));
}

FText UEvadingLocationQueryTest::GetDescriptionDetails() const
{
	return DescribeFloatTestParams();
}
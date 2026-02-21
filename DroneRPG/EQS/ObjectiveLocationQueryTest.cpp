#include "ObjectiveLocationQueryTest.h"
#include "AIController.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "DroneRPG/DroneRPGCharacter.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"

#define mSphereTraceMultiEQS(start, end, radius, hits) UKismetSystemLibrary::SphereTraceMulti(GetWorld(), start, end, radius, ETraceTypeQuery::TraceTypeQuery1, true, ignore, EDrawDebugTrace::None, hits, true);

UObjectiveLocationQueryTest::UObjectiveLocationQueryTest(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{	// Set our ItemType to a vector, as the items (grid points) we're looking for, are locations in the world
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();

	// Set Filter Type to range, this will then be a curve of values which allows us to have scores from 0 to 1. I need to look into this more to understand it better
	FilterType = EEnvTestFilterType::Range;

	// If we're filtering only, then we can only pass or fail locations, and scoring only just manipulates the current scores of items.
	// We want to do both, as we need to filter out of range and out of line of sight items but also score in range and in line of sight items higher, the closer they are to our current location
	TestPurpose = EEnvTestPurpose::Filter;

	// Scores that are higher considered better
	ScoringEquation = EEnvTestScoreEquation::Constant;
}

bool UObjectiveLocationQueryTest::CanNavigateToPoint(ACharacter* character, const FVector& actorLocation, const FVector& ItemLocation) const
{
	bool canNavigate = false;
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		UNavigationPath* Path = NavSys->FindPathToLocationSynchronously(GetWorld(), actorLocation, ItemLocation, character);

		// We have a path to the target
		if (Path && Path->IsValid() && !Path->IsPartial())
		{
			canNavigate = true;
		}
	}
	return  canNavigate;
}

void UObjectiveLocationQueryTest::RunTest(FEnvQueryInstance& QueryInstance) const
{
	//Super::RunTest(QueryInstance);

	// Query Owner will be the controller that ran the query
	AAIController* QueryOwner = Cast<AAIController>(QueryInstance.Owner.Get());
	if (!IsValid(QueryOwner))
	{
		return;
	}

	TArray<FVector> ContextLocations;

	// This will get the locations created by our context, in our case it's a single location based on the AI Controllers GetLastKnowLocation
	if (!QueryInstance.PrepareContext(this->Context, ContextLocations))
	{
		return;
	}

	FVector closetLocation = FVector::ZeroVector;
	FVector contextLocation = ContextLocations[0];

	ACharacter* character = QueryOwner->GetCharacter();
	FVector actorLocation = character->GetActorLocation();

	if (contextLocation != FVector::ZeroVector)
	{
		for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
		{
			// Get the current item as a FVector
			FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
			UNavigationSystemV1::K2_ProjectPointToNavigation(GetWorld(), ItemLocation, ItemLocation, nullptr, nullptr);

			//if (!CanNavigateToPoint(character, actorLocation, ItemLocation)) 
			//{
			//	It.ForceItemState(EEnvItemStatus::Failed);
			//	continue;
			//}

			// Do we have clear Line of sight to the location?
			//if (CanSeePoint(contextLocation, ItemLocation))
			//{
			//	//closetLocation = UFunctionLibrary::GetClosestLocation(ItemLocation, closetLocation, QueryOwner->GetOwner()->GetActorLocation());
			//	// Set the location as passed
			//	It.ForceItemState(EEnvItemStatus::Passed);
			//}
			//// We don't have line of Sight to the target
			//else
			//{
			It.ForceItemState(EEnvItemStatus::Failed);
			//}
		}
	}
}

FText UObjectiveLocationQueryTest::GetDescriptionTitle() const
{
	return FText::FromString(FString::Printf(TEXT("Objective location Test")));
}

FText UObjectiveLocationQueryTest::GetDescriptionDetails() const
{
	return DescribeFloatTestParams();
}

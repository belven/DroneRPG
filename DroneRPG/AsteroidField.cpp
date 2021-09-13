#include "AsteroidField.h"
#include "FunctionLibrary.h"
#include <Kismet/GameplayStatics.h>
#include "NavigationSystem.h"
#include <EngineUtils.h>
#include <Kismet/KismetMathLibrary.h>
#include "Objective.h"

#define mAddComponentByClass(classType, trans) Cast<classType>(AddComponentByClass(classType::StaticClass(), false, trans, false));

AAsteroidField::AAsteroidField()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> asteroidOne(TEXT("StaticMesh'/Game/TopDownCPP/Models/Terrain/Asteroids/Asteriod_1.Asteriod_1'"));

	if (asteroidOne.Succeeded())
		meshes.Add(asteroidOne.Object, 100);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> asteroidTwo(TEXT("StaticMesh'/Game/TopDownCPP/Models/Terrain/Asteroids/Asteriod_2.Asteriod_2'"));

	if (asteroidTwo.Succeeded())
		meshes.Add(asteroidTwo.Object, 100);

	PrimaryActorTick.bCanEverTick = false;
	minDist = 300;
	radius = 5000;

	minScale = 1.0f;
	maxScale = 2.0f;
}

void AAsteroidField::BeginPlay()
{
	Super::BeginPlay();

	for (auto& pair : meshes) {
		for (int32 i = 0; i < pair.Value; i++) {
			SpawnAsteroid(pair.Key);
		}
	}

	TArray<AObjective*> objectives = mGetActorsInWorld<AObjective>(GetWorld());
	
	for (AObjective* objective : objectives)
	{
		objective->ClearOutOverlap(this);
	}
}

void AAsteroidField::SpawnAsteroid(UStaticMesh* mesh) {
	FVector loc = GetAsteroidLocation();

	if (!loc.IsNearlyZero()) {
		float rand = FMath::RandRange(minScale, maxScale);
		float x = FMath::RandRange(rand * 0.9f, rand * 1.1f);
		float y = FMath::RandRange(rand * 0.9f, rand * 1.1f);
		float z = FMath::RandRange(rand * 0.9f, rand * 1.1f);

		FTransform trans(UKismetMathLibrary::RandomRotator(), FVector::ZeroVector, FVector(x, y, z));

		UStaticMeshComponent* asteroidMesh = mAddComponentByClass(UStaticMeshComponent, trans);

		asteroidMesh->SetStaticMesh(mesh);
		asteroidMesh->SetWorldLocation(loc);
	}
}

FVector AAsteroidField::GetAsteroidLocation() {
	FNavLocation loc;
	bool locTooClose = true;
	int32 count = 0;

	TArray<UStaticMeshComponent*> comps;
	GetComponents<UStaticMeshComponent>(comps);

	mRandomReachablePointInRadius(GetActorLocation(), radius, loc);

	while (locTooClose && count < 20) {
		locTooClose = false;
		count++;

		for (UStaticMeshComponent* comp : comps) {
			if (mDist(comp->GetComponentLocation(), loc) <= minDist) {
				locTooClose = true;
				mRandomReachablePointInRadius(GetActorLocation(), radius, loc);
				break;
			}
		}
	}

	if (count == 20) {
		return FVector::ZeroVector;
	}

	return loc;
}

void AAsteroidField::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
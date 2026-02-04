#include "AsteroidField.h"
#include "FunctionLibrary.h"
#include "NavigationSystem.h"
#include <Kismet/KismetMathLibrary.h>
#include "KeyActor.h"
#include "Components/InstancedStaticMeshComponent.h"

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
	minDist = 2000;
	radius = 20000;

	minScale = 1.0f;
	maxScale = 2.0f;
}

void AAsteroidField::RemoveOverlappingComponents(AActor* other, float size) {
	TArray<UInstancedStaticMeshComponent*> comps;
	GetComponents<UInstancedStaticMeshComponent>(comps);

	for (UInstancedStaticMeshComponent* comp : comps) {
		for (int32 i = 0; i < comp->GetInstanceCount() - 1; i++) {
			FTransform trans;
			float dist = MAX(minDist, size);

			comp->GetInstanceTransform(i, trans);
			if (IsAsteroidTooClose(trans, other->GetActorLocation(), size)) {
				comp->RemoveInstance(i);
			}
		}
	}
}

void AAsteroidField::ClearOutOverlap(AActor* other, float size) {
	RemoveOverlappingComponents(other, size);
}

void AAsteroidField::BeginPlay()
{
	Super::BeginPlay();

	// Create all the asteroids
	for (auto& pair : meshes) {
		for (int32 i = 0; i < pair.Value; i++) {
			SpawnAsteroid(pair.Key);
		}
	}

	// Make sure we don't overlap with a KeyActor, like a respawn point or objective
	TArray<AKeyActor*> keyActors = mGetActorsInWorld<AKeyActor>(GetWorld());

	for (AKeyActor* keyActor : keyActors)
	{
		ClearOutOverlap(keyActor, keyActor->GetSize());
	}
}

void AAsteroidField::SpawnAsteroid(UStaticMesh* mesh) {
	UInstancedStaticMeshComponent* comp;

	// Create a UInstancedStaticMeshComponent for each type of mesh, this is more efficient later on
	if (!meshInstances.Contains(mesh)) {
		comp = NewObject<UInstancedStaticMeshComponent>(this);
		comp->SetStaticMesh(mesh);
		comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		comp->SetupAttachment(RootComponent);
		comp->RegisterComponent();
		meshInstances.Add(mesh, comp);
	}
	else {
		comp = *meshInstances.Find(mesh);
	}

	FVector loc = GetAsteroidLocation();

	// If we found a location, spawn an mesh
	if (!loc.IsNearlyZero()) {

		// Get a random fixed scale
		float rand = FMath::RandRange(minScale, maxScale);

		// Limit the scale differences by no more than 10% smaller or larger
		float min = rand * 0.9f;
		float max = rand * 1.1f;

		// set up scale values within a random range of min max
		float x = FMath::RandRange(min, max);
		float y = FMath::RandRange(min, max);
		float z = FMath::RandRange(min, max);

		FTransform trans(UKismetMathLibrary::RandomRotator(), loc, FVector(x, y, z));

		comp->AddInstance(trans, true);
	}
}

bool AAsteroidField::IsAsteroidTooClose(const FTransform& asteroidTrans, const FVector& otherLoc, float inDist) {
	float dist = mDist(asteroidTrans.GetLocation(), otherLoc);

	// Adds 1/2 of the scale to the distance, as asteroidTrans.Location is the center and the asteroid could be very large
	float scaleMinDist = (1 + (asteroidTrans.GetMaximumAxisScale() / 20));
	float tooCloseDist = MAX(minDist, inDist) * scaleMinDist;
	return dist <= tooCloseDist;
}

FVector AAsteroidField::GetAsteroidLocation() {
	bool locTooClose = true;
	int32 count = 0;
	FNavLocation loc;

	// Get all other UInstancedStaticMeshComponent, to check we don't spawn too close to them
	TArray<UInstancedStaticMeshComponent*> comps;
	GetComponents<UInstancedStaticMeshComponent>(comps);

	// Get a random point
	mRandomReachablePointInRadius(GetActorLocation(), radius, loc);

	// Iterate over all other current UInstancedStaticMeshComponents, and check if we're too close
	// Keep checking a new location each time
	while (locTooClose && count < 20) {
		locTooClose = false;
		count++;

		for (UInstancedStaticMeshComponent* comp : comps) {
			for (int32 i = 0; i < comp->GetInstanceCount() - 1; i++) {
				FTransform trans;

				comp->GetInstanceTransform(i, trans);

				if (IsAsteroidTooClose(trans, loc, minDist)) {
					locTooClose = true;
					mRandomReachablePointInRadius(GetActorLocation(), radius, loc);
					break;
				}
			}
		}
	}

	if (count == 20) {
		return FVector::ZeroVector;
	}

	// Increase the Z so they spawn blocking line of sight for the drones
	loc.Location.Z += 350;

	return loc;
}

void AAsteroidField::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
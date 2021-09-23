#include "AsteroidField.h"
#include "FunctionLibrary.h"
#include <Kismet/GameplayStatics.h>
#include "NavigationSystem.h"
#include <EngineUtils.h>
#include <Kismet/KismetMathLibrary.h>
#include "Objective.h"
#include "RespawnPoint.h"

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

void AAsteroidField::RemoveOverlapingComponents(AActor* other) {
	TArray<UInstancedStaticMeshComponent*> comps;
	GetComponents<UInstancedStaticMeshComponent>(comps);

	for (UInstancedStaticMeshComponent* comp : comps) {
		for (int32 i = 0; i < comp->GetInstanceCount() - 1; i++) {
			FTransform trans;

			comp->GetInstanceTransform(i, trans);
			if (mDist(trans.GetLocation(), other->GetActorLocation()) < minDist) {
				comp->RemoveInstance(i);
			}
		}
	}
}

void AAsteroidField::ClearOutOverlap(AActor* other) {
	if (other != NULL) {
		RemoveOverlapingComponents(other);
	}
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
		ClearOutOverlap(objective);
	}

	TArray<ARespawnPoint*> respawnPoints = mGetActorsInWorld<ARespawnPoint>(GetWorld());

	for (ARespawnPoint* respawnPoint : respawnPoints)
	{
		ClearOutOverlap(respawnPoint);
	}
}

void AAsteroidField::SpawnAsteroid(UStaticMesh* mesh) {
	UInstancedStaticMeshComponent* comp;

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

	if (!loc.IsNearlyZero()) {
		float rand = FMath::RandRange(minScale, maxScale);
		float min = rand * 0.9f;
		float max = rand * 1.1f;
		float x = FMath::RandRange(min, max);
		float y = FMath::RandRange(min, max);
		float z = FMath::RandRange(min, max);

		FTransform trans(UKismetMathLibrary::RandomRotator(), loc, FVector(x, y, z));

		comp->AddInstanceWorldSpace(trans);
	}
}

FVector AAsteroidField::GetAsteroidLocation() {
	bool locTooClose = true;
	int32 count = 0;
	FNavLocation loc;

	TArray<UInstancedStaticMeshComponent*> comps;
	GetComponents<UInstancedStaticMeshComponent>(comps);

	mRandomReachablePointInRadius(GetActorLocation(), radius, loc);

	while (locTooClose && count < 20) {
		locTooClose = false;
		count++;

		for (UInstancedStaticMeshComponent* comp : comps) {
			for (int32 i = 0; i < comp->GetInstanceCount() - 1; i++) {
				FTransform trans;

				comp->GetInstanceTransform(i, trans);

				if (mDist(trans.GetLocation(), loc) <= minDist) {
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

	return loc;
}

void AAsteroidField::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
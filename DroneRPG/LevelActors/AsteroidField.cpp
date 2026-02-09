#include "AsteroidField.h"
#include <Kismet/KismetMathLibrary.h>
#include "KeyActor.h"
#include "NavigationSystem.h"
#include "Objective.h"
#include "RespawnPoint.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "DroneRPG/Utilities/FunctionLibrary.h"

#define mAddComponentByClass(classType, trans) Cast<classType>(AddComponentByClass(classType::StaticClass(), false, trans, false));

AAsteroidField::AAsteroidField() : objectives(0), teams(0)
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> asteroidOne(TEXT("StaticMesh'/Game/TopDownCPP/Models/Terrain/Asteroids/Asteriod_1.Asteriod_1'"));

	if (asteroidOne.Succeeded())
		meshes.Add(asteroidOne.Object);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> asteroidTwo(TEXT("StaticMesh'/Game/TopDownCPP/Models/Terrain/Asteroids/Asteriod_2.Asteriod_2'"));

	if (asteroidTwo.Succeeded())
		meshes.Add(asteroidTwo.Object);

	PrimaryActorTick.bCanEverTick = false;
	minDist = 2000;
	radius = 20000;

	minScale = 1.0f;
	maxScale = 2.0f;

	asteroids = 30;
}

void AAsteroidField::RemoveOverlappingComponents(AActor* other, float size) {
	TArray<UInstancedStaticMeshComponent*> comps;
	GetComponents<UInstancedStaticMeshComponent>(comps);

	for (UInstancedStaticMeshComponent* comp : comps) {
		for (int32 i = 0; i < comp->GetInstanceCount() - 1; i++) {
			FTransform trans;
			float dist = MAX(minDist, size);

			comp->GetInstanceTransform(i, trans);
			if (IsAsteroidTooClose(trans, other->GetActorLocation(), dist)) {
				comp->RemoveInstance(i);
			}
		}
	}
}

void AAsteroidField::ClearOutOverlap(AActor* other, float size) {
	RemoveOverlappingComponents(other, size);
}

void AAsteroidField::PushVectors(TMap<int32, FNode>& locations, const FVector& originPoint)
{
	for (auto& Elem : locations)
	{
		if (Elem.Value.type == ENodeType::Asteroid || Elem.Value.type == ENodeType::Objective) {
			FVector& Location = Elem.Value.location;

			FVector Direction = Location - originPoint;

			if (!Direction.IsNearlyZero())
			{
				Direction.Normalize();
				Location = originPoint + Direction * minDist;
				Location.Z = GetActorLocation().Z;
			}
		}
	}
}

void AAsteroidField::ClearUpAroundKeyActors()
{
	// Make sure we don't overlap with a KeyActor, like a respawn point or objective
	TArray<AKeyActor*> keyActors = UFunctionLibrary::GetActorsInWorld<AKeyActor>(GetWorld());

	for (AKeyActor* keyActor : keyActors)
	{
		ClearOutOverlap(keyActor, keyActor->GetSize());
	}
}

void AAsteroidField::SpaceTooCloseVectors(TMap<int32, FNode>& locations)
{
	for (auto& locationA : locations) {
		for (auto& locationB : locations) {
			if (locationA.Key == locationB.Key) continue;

			ENodeType locationAType = locationA.Value.type;
			if ((locationAType == ENodeType::Asteroid || locationAType == ENodeType::Objective)
				&& locationB.Value.type != ENodeType::SpawnPoint && locationAType != ENodeType::SpawnPoint) {
				FVector Delta = locationA.Value.location - locationB.Value.location;
				float Dist = Delta.Size();

				if (Dist < minDist && Dist > KINDA_SMALL_NUMBER)
				{
					locationA.Value.location += Delta.GetSafeNormal() * (minDist - Dist) * 0.5f;
				}
			}
		}
	}
}

void AAsteroidField::CreateSpawnPoint(TArray<ARespawnPoint*>& respawnPoints, int32 team, const FVector& newLocation)
{
	ARespawnPoint* respawn = GetWorld()->SpawnActor<ARespawnPoint>(ARespawnPoint::StaticClass(), newLocation, newLocation.Rotation());
	respawn->SetTeam(team);
	respawn->SetupParticles();
	respawn->SetTeamSize(5);
	respawnPoints.Add(respawn);
}

void AAsteroidField::CreateObjective(const FVector& centre)
{
	GetWorld()->SpawnActor<AObjective>(AObjective::StaticClass(), centre, centre.Rotation());
}

void AAsteroidField::BeginPlay()
{
	Super::BeginPlay();

	int32 lastID = 0;
	TMap<int32, FNode> nodes;
	TArray<ARespawnPoint*> respawnPoints;

	const float distance = radius * .4;

	const FVector centre(0.f, 0.f, 0.f);

	CreateObjective(centre);
	nodes.Add(lastID++, FNode(centre, ENodeType::Objective));

	for (int32 i = 0; i < teams; ++i)
	{
		const float angleStep = 360.f / teams;
		const float angleDeg = i * angleStep;
		const float angleRad = FMath::DegreesToRadians(angleDeg);

		const FVector newLocation(centre.X + FMath::Cos(angleRad) * distance, centre.Y + FMath::Sin(angleRad) * distance, centre.Z + 500);

		nodes.Add(lastID++, FNode(newLocation, ENodeType::SpawnPoint));
	}

	for (int i = 0; i < objectives; ++i)
	{
		const float angleStep = 360.f / objectives;
		const float angleDeg = i * angleStep * FMath::RandRange(0.8, 1.2);
		const float angleRad = FMath::DegreesToRadians(angleDeg);

		FVector newLocation(centre.X + FMath::Cos(angleRad) * distance, centre.Y + FMath::Sin(angleRad) * distance, centre.Z + 500);

		for (auto node : nodes)
		{
			if (FVector::Dist(node.Value.location, newLocation) <= minDist)
			{
				//newLocation.X -= 50;
				//newLocation.Y -= 50;
				FVector Delta = newLocation - centre;
		//		float Dist = Delta.Size();
				newLocation -= Delta.GetSafeNormal() * minDist;
			}
		}

		nodes.Add(lastID++, FNode(newLocation, ENodeType::Objective));
	}

	float radius1 = radius * .5;
	for (int x = -radius1; x < radius1; x += minDist)
	{
		for (int y = -radius1; y < radius1; y += minDist)
		{
			FVector location = FVector(x, y, centre.Z + 500);
			bool tooClose = false;

			for (auto node : nodes)
			{
				if ((node.Value.type == ENodeType::Objective || node.Value.type == ENodeType::SpawnPoint)
					&& FVector::Dist(node.Value.location, location) <= minDist)
				{
					tooClose = true;
					break;
				}
			}

			if (!tooClose) {
				nodes.Add(lastID++, FNode(location, ENodeType::Asteroid));
			}
		}
	}

	int32 team = 0;
	// Create all the asteroids
	for (auto& location : nodes) {

		if (location.Value.type == ENodeType::SpawnPoint)
		{
			CreateSpawnPoint(respawnPoints, team++, location.Value.location);
		}
		else if (location.Value.type == ENodeType::Objective)
		{
			CreateObjective(location.Value.location);
		}
		else {
			UStaticMesh* mesh = UFunctionLibrary::GetRandomObject(meshes);
			SpawnAsteroid(mesh, location.Value.location);
		}
	}

	//	ClearUpAroundKeyActors();

	for (ARespawnPoint* respawnPoint : respawnPoints)
	{
		respawnPoint->SpawnTeam();
	}
}

void AAsteroidField::SpawnAsteroid(UStaticMesh* mesh, const FVector& location) {
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

	// If we found a location, spawn an mesh
	if (!location.IsNearlyZero()) {

		// Get a random fixed scale
		float rand = FMath::RandRange(minScale, maxScale);

		// Limit the scale differences by no more than 10% smaller or larger
		float min = rand * 0.9f;
		float max = rand * 1.1f;

		// set up scale values within a random range of min max
		float x = FMath::RandRange(min, max);
		float y = FMath::RandRange(min, max);
		float z = FMath::RandRange(min, max);

		FTransform trans(UKismetMathLibrary::RandomRotator(), location, FVector(x, y, z));

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
	FNavLocation loc;

	// Get a random point
	mRandomReachablePointInRadius(GetActorLocation(), radius, loc);
	loc.Location.Z = GetActorLocation().Z;
	loc.Location.Z += 600;

	return loc;
}

void AAsteroidField::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
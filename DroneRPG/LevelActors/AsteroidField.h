#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AsteroidField.generated.h"

class ARespawnPoint;

UENUM(BlueprintType)
enum class  ENodeType : uint8 {
	Asteroid,
	SpawnPoint,
	Objective,
	End
};

USTRUCT(BlueprintType)
struct FNode
{
	GENERATED_USTRUCT_BODY()

	FNode(): location(), type()
	{
	}

	FNode(const FVector& inLocation, ENodeType inType)
		: location(inLocation),
		type(inType)
	{
	}

	FVector location;
	ENodeType type;
};

UCLASS()
class DRONERPG_API AAsteroidField : public AActor
{
	GENERATED_BODY()

public:
	AAsteroidField();

	void RemoveOverlappingComponents(AActor* other, float size = 0);
	void ClearOutOverlap(AActor* other, float size = 0);
	void PushVectors(TMap<int32, FNode>& locations, const FVector& originPoint);
	void SpaceTooCloseVectors(TMap<int32, FNode>& locations);
	void ClearUpAroundKeyActors();
	void CreateSpawnPoint(TArray<ARespawnPoint*>& respawnPoints, int32 team, const FVector& newLocation);
	void CreateObjective(const FVector& centre);
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid Field")
	float  radius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid Field")
	float  asteroids;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid Field")
	float  objectives;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid Field")
	float  minDist;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid Field")
	float  minScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid Field")
	float  maxScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid Field")
	float  teams;

protected:
	virtual void BeginPlay() override;

	void SpawnAsteroid(UStaticMesh* mesh, const FVector& location);
	bool IsAsteroidTooClose(const FTransform& asteroidTrans, const FVector& otherLoc, float inDist);
	FVector GetAsteroidLocation();
private:
	UPROPERTY()
	TArray<UStaticMesh*> meshes;
	UPROPERTY()
	TMap<UStaticMesh*, UInstancedStaticMeshComponent*> meshInstances;
};

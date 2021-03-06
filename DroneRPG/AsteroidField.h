// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AsteroidField.generated.h"

UCLASS()
class DRONERPG_API AAsteroidField : public AActor
{
	GENERATED_BODY()

public:
	AAsteroidField();

	void RemoveOverlapingComponents(AActor* other, float size = 0);
	void ClearOutOverlap(AActor* other, float size = 0);
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid Field")
		float  radius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid Field")
		float  minDist;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid Field")
		float  minScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid Field")
		float  maxScale;

protected:
	virtual void BeginPlay() override;

	void SpawnAsteroid(UStaticMesh* mesh);
	bool IsAsteroidTooClose(FTransform asteroidTrans, FVector otherLoc, float inDist);
	FVector GetAsteroidLocation();
private:
	TMap<UStaticMesh*, int32> meshes;
	TMap<UStaticMesh*, UInstancedStaticMeshComponent*> meshInstances;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KeyActor.generated.h"

UCLASS()
class DRONERPG_API AKeyActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AKeyActor();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeyActor")
	float keyActorSize;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	float GetSize() const { return keyActorSize; }
	void SetSize(float val) { keyActorSize = val; }
};

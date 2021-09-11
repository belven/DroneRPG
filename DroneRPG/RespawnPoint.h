// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RespawnPoint.generated.h"

class ADroneRPGCharacter;

UCLASS()
class DRONERPG_API ARespawnPoint : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARespawnPoint();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void RespawnCharacter(ADroneRPGCharacter* character);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		int32 team;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	int32 GetTeam() const { return team; }
	void SetTeam(int32 val) { team = val; }
};
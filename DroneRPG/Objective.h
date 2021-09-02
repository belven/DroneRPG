// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <NiagaraComponent.h>
#include "Objective.generated.h"

class UBoxComponent;
class ADroneRPGCharacter;
class UNiagaraSystem;

UCLASS()
class DRONERPG_API AObjective : public AActor
{
	GENERATED_BODY()

public:
	AObjective();

	UFUNCTION()
		void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Effects)
		UNiagaraComponent* captureParticle;

	int32 GetAreaOwner() const { return areaOwner; }
	void SetAreaOwner(int32 val) { areaOwner = val; }

	bool HasCompleteControl(int32 team);

	FName GetObjectiveName() const { return objectiveName; }
	void SetObjectiveName(FName val) { objectiveName = val; }
protected:
	virtual void BeginPlay() override;

	void CalculateOwnership();
	void UpdateColour();
	void CalculateClaim();
private:
	UPROPERTY()
		UBoxComponent* objectiveArea;

	UPROPERTY()
		TArray<ADroneRPGCharacter*> dronesInArea;

	UPROPERTY()
		int32 areaOwner;

	UPROPERTY()
		int32 currentControl;

	UPROPERTY()
		FColor currentColour;

	UPROPERTY()
	bool fullClaim;

	UPROPERTY()
		FName objectiveName;

	UNiagaraSystem* auraSystem;

};

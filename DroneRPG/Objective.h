// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <NiagaraComponent.h>
#include "Objective.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectiveClaimed, AObjective*, objective);

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
	void SetAreaOwner(int32 val);

	bool HasCompleteControl(int32 team);

	FObjectiveClaimed OnObjectiveClaimed;

	FName GetObjectiveName() const { return objectiveName; }
	void SetObjectiveName(FName val) { objectiveName = val; }
protected:
	virtual void BeginPlay() override;

	void CalculateOwnership();
	void UpdateColour();
	void CalculateClaim();
	template<class T> T ClampValue(T value, T max, T min);


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
		int32 areaOwner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
		int32 preiviousAreaOwner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
		float currentControl;

private:
	UPROPERTY()
		UBoxComponent* objectiveArea;

	UPROPERTY()
		TArray<ADroneRPGCharacter*> dronesInArea;


	int32 minControl;
	int32 maxControl;

	UPROPERTY()
		TArray<int32> teamsInArea;

	UPROPERTY()
		FColor currentColour;

	UPROPERTY()
		bool fullClaim;

	UPROPERTY()
		FName objectiveName;

	UNiagaraSystem* auraSystem;

};

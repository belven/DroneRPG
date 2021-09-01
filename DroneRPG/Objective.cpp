// Fill out your copyright notice in the Description page of Project Settings.


#include "Objective.h"
#include "Components/BoxComponent.h"
#include "DroneRPGCharacter.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "../Plugins/FX/Niagara/Source/Niagara/Classes/NiagaraSystem.h"

#define  mIsA(aObject, aClass)  aObject->IsA(aClass::StaticClass())

// Sets default values
AObjective::AObjective()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	objectiveArea = CreateDefaultSubobject<UBoxComponent>(TEXT("ObjectiveArea"));
	objectiveArea->SetBoxExtent(FVector(700, 700, 100));
	objectiveArea->SetupAttachment(GetRootComponent());
	objectiveArea->SetRelativeLocation(FVector(0, 0, 50)); // TODO not working

	SetAreaOwner(0);
	currentControl = 75;
	currentColour = FColor::Red;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> auraParticleSystem(TEXT("/Game/TopDownCPP/ParticleEffects/AuraSystem"));

	if (auraParticleSystem.Succeeded()) {
		auraSystem = auraParticleSystem.Object;
	}
}

void AObjective::BeginOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult) {

	if (mIsA(OtherActor, ADroneRPGCharacter) && !dronesInArea.Contains(OtherActor)) {
		dronesInArea.Add(Cast<ADroneRPGCharacter>(OtherActor));
		CalculateOwnership();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("A drone entered the area")));
	}
}

void AObjective::EndOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (mIsA(OtherActor, ADroneRPGCharacter) && dronesInArea.Contains(OtherActor)) {
		dronesInArea.Remove(Cast<ADroneRPGCharacter>(OtherActor));
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("A drone left the area")));
		CalculateOwnership();
	}
}

// Called when the game starts or when spawned
void AObjective::BeginPlay()
{
	Super::BeginPlay();
	objectiveArea->OnComponentBeginOverlap.AddDynamic(this, &AObjective::BeginOverlap);
	objectiveArea->OnComponentEndOverlap.AddDynamic(this, &AObjective::EndOverlap);

	captureParticle = UNiagaraFunctionLibrary::SpawnSystemAttached(auraSystem, RootComponent, TEXT("captureParticle"), FVector(1), FRotator(1), EAttachLocation::SnapToTarget, false);

	captureParticle->SetFloatParameter(TEXT("Radius"), 400);
	captureParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Red));
	captureParticle->SetFloatParameter(TEXT("Size"), 35);
}

void AObjective::CalculateOwnership() {
	bool team1 = false;
	bool team2 = false;

	for (ADroneRPGCharacter* drone : dronesInArea) {
		if (drone->GetTeam() == 1) {
			team1 = true;
		}
		else {
			team2 = true;
		}
	}

	if (team1 && !team2) {
		SetAreaOwner(1);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Area Owner is now Team 1")));
	}
	else if (!team1 && team2) {
		SetAreaOwner(2);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Area Owner is now Team 2")));
	}
	else {
		SetAreaOwner(0);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Area Owner is now no Team")));
	}

}

void AObjective::UpdateColour() {
	if (currentControl <= 50 && currentColour != FColor::Blue) {
		captureParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Blue));
		currentColour = FColor::Blue;
	}
	else if (currentControl >= 100 && currentColour != FColor::Green) {
		captureParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Green));
		currentColour = FColor::Green;
	}
	else if (currentColour != FColor::Red) {
		captureParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Red));
		currentColour = FColor::Red;
	}
}

// Called every frame
void AObjective::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetAreaOwner() == 1 && currentControl > 0) {
		currentControl--;
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Current Control %d"), currentControl));
		UpdateColour();
	}
	else	if (GetAreaOwner() == 2 && currentControl < 150) {
		currentControl++;
		UpdateColour();
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Current Control %d"), currentControl));
	}


}

bool AObjective::HasCompleteControl(int32 team)
{
	return (team == 1 && currentControl == 0) || (team == 2 && currentControl == 150);
}

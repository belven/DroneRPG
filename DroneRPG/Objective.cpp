#include "Objective.h"
#include "Components/BoxComponent.h"
#include "DroneRPGCharacter.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

#define  mIsA(aObject, aClass)  aObject->IsA(aClass::StaticClass())
#define  mAddOnScreenDebugMessage(text) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT(text)));

AObjective::AObjective()
{
	PrimaryActorTick.bCanEverTick = true;
	objectiveArea = CreateDefaultSubobject<UBoxComponent>(TEXT("ObjectiveArea"));
	objectiveArea->SetBoxExtent(FVector(700, 700, 100));
	objectiveArea->SetRelativeLocation(FVector(0, 0, 50)); // TODO not working
	objectiveArea->SetupAttachment(GetRootComponent());

	SetAreaOwner(0);
	currentControl = 75;
	currentColour = FColor::Red;
	fullClaim = false;
	objectiveName = "";

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
	const FHitResult& SweepResult)
{
	if (mIsA(OtherActor, ADroneRPGCharacter) && !dronesInArea.Contains(OtherActor)) {
		dronesInArea.Add(Cast<ADroneRPGCharacter>(OtherActor));
		CalculateOwnership();
		mAddOnScreenDebugMessage("A drone entered the area");
	}
}

void AObjective::EndOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (mIsA(OtherActor, ADroneRPGCharacter) && dronesInArea.Contains(OtherActor)) {
		dronesInArea.Remove(Cast<ADroneRPGCharacter>(OtherActor));
		mAddOnScreenDebugMessage("A drone left the area");
		CalculateOwnership();
	}
}

void AObjective::BeginPlay()
{
	Super::BeginPlay();
	objectiveArea->OnComponentBeginOverlap.AddDynamic(this, &AObjective::BeginOverlap);
	objectiveArea->OnComponentEndOverlap.AddDynamic(this, &AObjective::EndOverlap);

	captureParticle = UNiagaraFunctionLibrary::SpawnSystemAttached(auraSystem, RootComponent, TEXT("captureParticle"), FVector(1), FRotator(1), EAttachLocation::SnapToTarget, false);

	captureParticle->SetFloatParameter(TEXT("Radius"), 400);
	captureParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Red));
	captureParticle->SetFloatParameter(TEXT("Size"), 25);
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
		mAddOnScreenDebugMessage("Area Owner is now Team 1");
	}
	else if (!team1 && team2) {
		SetAreaOwner(2);
		mAddOnScreenDebugMessage("Area Owner is now Team 2");
	}
	else {
		SetAreaOwner(0);
		mAddOnScreenDebugMessage("No Team owns the area");
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
	else if (currentControl < 100 && currentControl > 50 && currentColour != FColor::Red) {
		captureParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Red));
		currentColour = FColor::Red;
	}
}

void AObjective::CalculateClaim() {
	if (GetAreaOwner() == 1 && currentControl > 0) {
		currentControl--;
		UpdateColour();
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Current Control %d"), currentControl));
	}
	else	if (GetAreaOwner() == 2 && currentControl < 150) {
		currentControl++;
		UpdateColour();
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Current Control %d"), currentControl));
	}

	if ((currentControl == 0 || currentControl == 150) && !fullClaim) {
		captureParticle->SetFloatParameter(TEXT("Size"), 45);
		fullClaim = true;
	}
	else if ((currentControl != 0 || currentControl != 150) && fullClaim) {
		captureParticle->SetFloatParameter(TEXT("Size"), 25);
		fullClaim = false;
	}
}

void AObjective::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CalculateClaim();
}

bool AObjective::HasCompleteControl(int32 team)
{
	return (team == 1 && currentControl == 0) || (team == 2 && currentControl == 150);
}
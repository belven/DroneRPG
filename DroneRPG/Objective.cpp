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
	currentControl = 0;
	currentColour = FColor::Red;
	fullClaim = false;
	objectiveName = "";

	minControl = 25;
	maxControl = 100;

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
	teamsInArea.Empty();

	for (ADroneRPGCharacter* drone : dronesInArea) {
		if (!teamsInArea.Contains(drone->GetTeam())) {
			teamsInArea.Add(drone->GetTeam());
		}
	}

	if (teamsInArea.Num() == 1 && GetAreaOwner() != teamsInArea[0]) {
		SetAreaOwner(teamsInArea[0]);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Team %d owns the area"), GetAreaOwner()));

		if (OnObjectiveClaimed.IsBound()) {
			OnObjectiveClaimed.Broadcast(this);
		}
	}
}

void AObjective::UpdateColour() {
	if (currentControl > minControl&& priviousAreaOwner == areaOwner) {
		if (areaOwner == 1 && currentColour != FColor::Green) {
			captureParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Green));
			currentColour = FColor::Green;
		}
		else if (areaOwner == 2 && currentColour != FColor::Yellow) {
			captureParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Yellow));
			currentColour = FColor::Yellow;
		}
	}
	else if (currentControl <= minControl && currentColour != FColor::Red) {
		captureParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Red));
		currentColour = FColor::Red;
	}
}
void AObjective::SetAreaOwner(int32 val)
{
	priviousAreaOwner = areaOwner;
	areaOwner = val;
}

void AObjective::CalculateClaim() {
	if (teamsInArea.Num() == 1) {
		if (priviousAreaOwner == 0 && areaOwner != 0) {
			currentControl++;
			UpdateColour();

			if (currentControl >= minControl) {
				priviousAreaOwner = areaOwner;
			}
		}
		else if (priviousAreaOwner != areaOwner && currentControl > 0) {
			currentControl--;
			UpdateColour();

			if (currentControl == 0)
				priviousAreaOwner = areaOwner;
		}
		else if (priviousAreaOwner == areaOwner && currentControl < maxControl) {
			currentControl++;
			UpdateColour();
		}

		if (currentControl > minControl && !fullClaim) {
			captureParticle->SetFloatParameter(TEXT("Size"), 45);
			fullClaim = true;

			if (OnObjectiveClaimed.IsBound()) {
				OnObjectiveClaimed.Broadcast(this);
			}
		}
		else if (currentControl <= minControl && fullClaim) {
			captureParticle->SetFloatParameter(TEXT("Size"), 25);
			fullClaim = false;
		}
	}
}

void AObjective::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CalculateClaim();
}

bool AObjective::HasCompleteControl(int32 team)
{
	return currentControl == maxControl && areaOwner == team;
}
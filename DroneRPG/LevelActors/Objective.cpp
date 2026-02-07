#include "Objective.h"
#include "Components/BoxComponent.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include <Components/StaticMeshComponent.h>
#include <Kismet/GameplayStatics.h>
#include "DroneRPG/DroneRPGCharacter.h"
#include "DroneRPG/FunctionLibrary.h"
#include "DroneRPG/GameModes/DroneRPGGameMode.h"

AObjective::AObjective()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1;
	areaOwner = 0;
	currentControl = 0;
	minControl = 3;
	maxControl = 10;
	fullClaim = false;
	currentColour = FColor::Red;

	keyActorSize = 2000;

	smallParticle = 100;
	bigParticle = 150;

	objectiveName = "";

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> auraParticleSystem(TEXT("/Game/TopDownCPP/ParticleEffects/AuraSystem_2"));

	if (auraParticleSystem.Succeeded()) {
		auraSystem = auraParticleSystem.Object;
	}

	objectiveArea = CreateDefaultSubobject<UBoxComponent>(TEXT("ObjectiveArea"));
	objectiveArea->SetBoxExtent(FVector(GetSize(), GetSize(), 400));
	objectiveArea->SetupAttachment(GetRootComponent());
}

void AObjective::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if we have a drone and we don't already have it in the list
	if (mIsA(OtherActor, ADroneRPGCharacter)) {
		// Add it to the list and re-calculate ownership
		Add(Cast<ADroneRPGCharacter>(OtherActor));
	}
}

void AObjective::DroneDied(ADroneRPGCharacter* drone) {
	Remove(drone);
}

void AObjective::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Check if we have a drone and we have it in the list
	if (mIsA(OtherActor, ADroneRPGCharacter)) {
		// Remove it from the list and re-calculate ownership
		Remove(Cast<ADroneRPGCharacter>(OtherActor));
	}
}

void AObjective::BeginPlay()
{
	Super::BeginPlay();

	// Bind to the box components begin and end overlap events
	objectiveArea->OnComponentBeginOverlap.AddDynamic(this, &AObjective::BeginOverlap);
	objectiveArea->OnComponentEndOverlap.AddDynamic(this, &AObjective::EndOverlap);

	// Create our particle system
	captureParticle = UNiagaraFunctionLibrary::SpawnSystemAttached(auraSystem, RootComponent, TEXT("captureParticle"), FVector(1), FRotator(1), EAttachLocation::SnapToTarget, false);

	// Set up the systems defaults
	captureParticle->SetVectorParameter(TEXT("Box Extent"), FVector(GetSize(), GetSize(), 400));
	captureParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Red));
	captureParticle->SetFloatParameter(TEXT("Size"), smallParticle);

	CalculateOwnership();

	// Update the colour in here, as we may have started with a team controlling us, set in the editor etc.
	UpdateColour();
}

void AObjective::CalculateOwnership() {
	// Clear the teams list, as we're calculating it again 
	teamsInArea.Empty();

	for (ADroneRPGCharacter* drone : dronesInArea) {
		if (!teamsInArea.Contains(drone->GetTeam()) && drone->IsAlive()) {
			teamsInArea.Add(drone->GetTeam());
		}
	}

	// If there's only 1 team in the area, then they have full claim of it
	if (teamsInArea.Num() == 1 && GetAreaOwner() != teamsInArea[0]) {
		SetAreaOwner(teamsInArea[0]);
	}
}

void AObjective::UpdateColour() {
	// Check if we have exceeded the minimum control value, if so then we can change the colour to the owning team
	// Check if the priviousAreaOwner and areaOwner are the same, this means the colour can change as the preiviousAreaOwner isn't an enemy team
	if (currentControl > minControl&& previousAreaOwner == areaOwner) {
		FColor teamColour = UFunctionLibrary::GetTeamColour(areaOwner);

		if (currentColour != teamColour) {
			captureParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(teamColour));
			currentColour = teamColour;
		}
	}
	// If the control is less than the minimum then it's a neutral 
	else if (currentControl <= minControl && currentColour != FColor::Red) {
		captureParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Red));
		currentColour = FColor::Red;
	}
}

void AObjective::SetAreaOwner(int32 val)
{
	areaOwner = val;
}

void AObjective::CalculateClaim() {

	// If only one team is in the area, then they  can start to claim it
	if (teamsInArea.Num() == 1) {

		// If the preiviousAreaOwner is 0 and there's a new owner then start to claim, this is only ever the case if it's yet to be claimed 
		if (previousAreaOwner == 0 && areaOwner != 0) {
			currentControl += dronesInArea.Num();
			currentControl = mClampValue<int32>(currentControl, maxControl, 0);

			UpdateColour();

			// Once the control exceeds the minimum control, the new team can have control
			if (currentControl >= minControl) {
				previousAreaOwner = areaOwner;
			}
		}
		// If the area owner isn't the same as the last and the area has some control, start to remove the control from the existing team
		else if (previousAreaOwner != areaOwner && currentControl > 0) {
			currentControl -= dronesInArea.Num();
			currentControl = mClampValue<int32>(currentControl, maxControl, 0);

			UpdateColour();

			// If the control is now 0, then we've removed all existing control and can start to claim it
			if (currentControl == 0)
				previousAreaOwner = areaOwner;
		}
		// If the previousAreaOwner and areaOwner are the same, then that team has control and we can start claiming it
		// Check if we're not at the max control
		else if (previousAreaOwner == areaOwner && currentControl < maxControl) {
			currentControl += dronesInArea.Num();
			currentControl = mClampValue<int32>(currentControl, maxControl, 0);

			UpdateColour();
		}

		// Check if we have full control and we've not already got full claim
		// If we have this level of control, the make the particles bigger
		if (currentControl == maxControl && !fullClaim) {
			captureParticle->SetFloatParameter(TEXT("Size"), bigParticle);
			fullClaim = true;

			if (OnObjectiveClaimed.IsBound()) {
				OnObjectiveClaimed.Broadcast(this);
			}
		}
		// If the control is less than max then make the particles smaller, this makes it easier to tell when it's fully claimed
		else if (currentControl < maxControl && fullClaim) {
			captureParticle->SetFloatParameter(TEXT("Size"), smallParticle);
			fullClaim = false;
		}
	}
}

void AObjective::Add(ADroneRPGCharacter* drone)
{
	if (!dronesInArea.Contains(drone) && drone->IsAlive()) {
		dronesInArea.Add(drone);
		drone->DroneDied.AddDynamic(this, &AObjective::DroneDied);
		CalculateOwnership();
	}
}

void AObjective::Remove(ADroneRPGCharacter* drone)
{
	if (dronesInArea.Contains(drone)) {
		dronesInArea.Remove(drone);
		drone->DroneDied.RemoveDynamic(this, &AObjective::DroneDied);
		CalculateOwnership();
	}
}

void AObjective::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CalculateClaim();

	// Every second add 5 points to the team that full owns this point
	if (fullClaim) {
		ADroneRPGGameMode* gm =  Cast<ADroneRPGGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		gm->AddTeamScore(areaOwner, 5);
	}
}

bool AObjective::HasCompleteControl(int32 team)
{
	return fullClaim && areaOwner == team;
}

float AObjective::GetCurrentControlPercent()
{
	return (currentControl / maxControl) * 100;
}

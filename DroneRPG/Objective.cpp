#include "Objective.h"
#include "Components/BoxComponent.h"
#include "DroneRPGCharacter.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "FunctionLibrary.h"
#include <Components/StaticMeshComponent.h>

AObjective::AObjective()
{
	PrimaryActorTick.bCanEverTick = true;
	objectiveArea = CreateDefaultSubobject<UBoxComponent>(TEXT("ObjectiveArea"));
	objectiveArea->SetBoxExtent(FVector(700, 700, 400));
	objectiveArea->SetRelativeLocation(FVector(0, 0, 50)); // TODO: not working
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
	// Check if we have a drone and we don't already have it in the list
	if (mIsA(OtherActor, ADroneRPGCharacter) && !dronesInArea.Contains(OtherActor)) {

		// Add it to the list and re-calculate ownership
		dronesInArea.Add(Cast<ADroneRPGCharacter>(OtherActor));
		CalculateOwnership();
		//mAddOnScreenDebugMessage("A drone entered the area");
	}
}

void AObjective::EndOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	// Check if we have a drone and we have it in the list
	if (mIsA(OtherActor, ADroneRPGCharacter) && dronesInArea.Contains(OtherActor)) {

		// Remove it from the list and re-calculate ownership
		dronesInArea.Remove(Cast<ADroneRPGCharacter>(OtherActor));
		//mAddOnScreenDebugMessage("A drone left the area");
		CalculateOwnership();
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
	captureParticle->SetFloatParameter(TEXT("Radius"), 400);
	captureParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Red));
	captureParticle->SetFloatParameter(TEXT("Size"), 25);

	// Update the colour in here, as we may have started with a team controlling us, set in the editor etc.
	UpdateColour();
	//ClearOutOverlap(NULL);
}

void AObjective::RemoveOverlapingComponents(AActor* other) {
	TArray<UStaticMeshComponent*> comps;
	other->GetComponents<UStaticMeshComponent>(comps);

	for (UStaticMeshComponent* comp : comps) {
		if (mDist(comp->GetComponentLocation(), GetActorLocation()) < 1500) {
			comp->SetHiddenInGame(true);
			comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AObjective::ClearOutOverlap(AActor* other) {
	if (other != NULL) {
		RemoveOverlapingComponents(other);
	}
	else {
		TArray<AActor*> actors;
		GetOverlappingActors(actors);

		for (AActor* a : actors) {
			RemoveOverlapingComponents(other);
		}
	}
}

void AObjective::CalculateOwnership() {
	// Clear the teams list, as we're calculating it again 
	teamsInArea.Empty();

	for (ADroneRPGCharacter* drone : dronesInArea) {
		if (!teamsInArea.Contains(drone->GetTeam()) && drone->IsAlive()) {
			teamsInArea.Add(drone->GetTeam());
		}
	}

	// IF there's only 1 team in the area, then they have full claim of it
	if (teamsInArea.Num() == 1 && GetAreaOwner() != teamsInArea[0]) {
		SetAreaOwner(teamsInArea[0]);
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Team %d owns the area"), GetAreaOwner()));
	}
}

void AObjective::UpdateColour() {
	// Check if we have exceeded the minimum control value, if so then we can change the colour to the owning team
	// Check if the priviousAreaOwner and areaOwner are the same, this means the colour can change as the preiviousAreaOwner isn't an enemy team
	if (currentControl > minControl&& preiviousAreaOwner == areaOwner) {

		// Set the colours correctly based on the team TODO: need to make a map of all the team numbers and their colours
		if (areaOwner == 1 && currentColour != FColor::Green) {
			captureParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Green));
			currentColour = FColor::Green;
		}
		else if (areaOwner == 2 && currentColour != FColor::Yellow) {
			captureParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Yellow));
			currentColour = FColor::Yellow;
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
		if (preiviousAreaOwner == 0 && areaOwner != 0) {
			currentControl += dronesInArea.Num();
			currentControl = mClampValue<int32>(currentControl, maxControl, 0);

			UpdateColour();

			// Once the control exceeds the minimum control, the new team can have control
			if (currentControl >= minControl) {
				preiviousAreaOwner = areaOwner;
			}
		}
		// If the area owner isn't the same as the last and the area has some control, start to remove the control from the existing team
		else if (preiviousAreaOwner != areaOwner && currentControl > 0) {
			currentControl -= dronesInArea.Num();
			currentControl = mClampValue<int32>(currentControl, maxControl, 0);

			UpdateColour();

			// If the control is now 0, then we've removed all existing control and can start to claim it
			if (currentControl == 0)
				preiviousAreaOwner = areaOwner;
		}
		// If the preiviousAreaOwner and areaOwner are the same, then that team has control and we can start claiming it
		// Check if we're not at the max control
		else if (preiviousAreaOwner == areaOwner && currentControl < maxControl) {
			currentControl += dronesInArea.Num();
			currentControl = mClampValue<int32>(currentControl, maxControl, 0);

			UpdateColour();
		}


		// Check if we have full control and we've not already got full claim
		// If we have this level of control, the make the particles bigger
		if (currentControl == maxControl && !fullClaim) {
			captureParticle->SetFloatParameter(TEXT("Size"), 45);
			fullClaim = true;

			if (OnObjectiveClaimed.IsBound()) {
				OnObjectiveClaimed.Broadcast(this);
			}
		}
		// If the control is less than max then make the particles smaller, this makes it easier to tell when it's fully claimed
		else if (currentControl < maxControl && fullClaim) {
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
	return fullClaim && areaOwner == team;
}
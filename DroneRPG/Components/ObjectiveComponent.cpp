#include "ObjectiveComponent.h"
#include "Components/SphereComponent.h"
#include "DroneRPG/Components/HealthComponent.h"
#include "DroneRPG/DroneRPG.h"
#include "DroneRPG/GameModes/DroneRPGGameMode.h"
#include "DroneRPG/Utilities/CombatClasses.h"
#include "DroneRPG/Utilities/FunctionLibrary.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include <DroneRPG/Components/CombatantComponent.h>
#include <Kismet/GameplayStatics.h>

FName UObjectiveComponent::RADIUS = TEXT("Radius");
FName UObjectiveComponent::COLOUR = TEXT("Colour");
FName UObjectiveComponent::SIZE = TEXT("Size");
FName UObjectiveComponent::PERCENT = TEXT("Percent");
FName UObjectiveComponent::ROTATION = TEXT("Rotation");

UObjectiveComponent::UObjectiveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1;
	objectiveName = "";

	scoreMultiplier = 1;
	areaOwner = 0;
	currentControl = 0;
	minControl = 3;
	maxControl = 10;
	fullClaim = false;
	setupComplete = false;
	currentColour = FColor::Red;

	smallParticle = 25;
	bigParticle = 50;
	overlapTimeRate = 5;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> auraParticleSystem(TEXT("/Script/Niagara.NiagaraSystem'/Game/TopDownCPP/ParticleEffects/ObjectiveCaptureParticlesWedge.ObjectiveCaptureParticlesWedge'"));

	if (auraParticleSystem.Succeeded())
	{
		auraSystem = auraParticleSystem.Object;
	}

	objectiveArea = CreateDefaultSubobject<USphereComponent>(TEXT("ObjectiveArea"));

	// Bind to the box components begin and end overlap events
	objectiveArea->OnComponentBeginOverlap.AddDynamic(this, &UObjectiveComponent::BeginOverlap);
	objectiveArea->OnComponentEndOverlap.AddDynamic(this, &UObjectiveComponent::EndOverlap);
	UFunctionLibrary::SetupOverlap(objectiveArea);

}

void UObjectiveComponent::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FCombatantData combatant = mCreateCombatantData(OtherActor);

	// Check if we have a drone and we have it in the list
	if (IsValid(combatant))
	{
		// Add it to the list and re-calculate ownership
		Add(combatant);
	}
}

void UObjectiveComponent::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	FCombatantData combatant = mCreateCombatantData(OtherActor);

	// Check if we have a drone and we have it in the list
	if (IsValid(combatant))
	{
		// Remove it from the list and re-calculate ownership
		Remove(combatant);
	}
}

void UObjectiveComponent::CheckForOverlaps()
{
	TArray<AActor*> overlaps;
	objectiveArea->GetOverlappingActors(overlaps);

	for (AActor* overlap : overlaps)
	{
		FCombatantData combatant = mCreateCombatantData(overlap);

		// Check if we have a drone and we have it in the list
		if (IsValid(combatant))
		{
			Add(combatant);
		}
	}
}

UNiagaraComponent* UObjectiveComponent::SpawnSystemAttached(FName name)
{
	return UNiagaraFunctionLibrary::SpawnSystemAttached(auraSystem, GetOwner()->GetRootComponent(), name, FVector(1), FRotator(1), EAttachLocation::SnapToTarget, false);
}

void UObjectiveComponent::UnitDied(AActor* unitKilled, UCombatantComponent* killer)
{
	Remove(mCreateCombatantData(unitKilled));
}

void UObjectiveComponent::SetTeamClaim(int32 team)
{
	SetAreaOwner(team);
	SetPreviousAreaOwner(team);
	SetAngle(GetCurrentTeamParticles(), 360);
	SetAngle(GetTransitioningParticles(), 0);
	SetCurrentControl(10);
	SetFullClaim(true);
	UpdateColour();
}

void UObjectiveComponent::SetupParticles(UNiagaraComponent** inNiagaraComponent, const FString& inStr, int32 angle)
{
	UNiagaraComponent* comp =	SpawnSystemAttached(FName(*inStr));
	*inNiagaraComponent = comp;
	comp->SetColorParameter(COLOUR, FLinearColor(FColor::Red));
	comp->SetFloatParameter(SIZE, smallParticle);
	comp->SetFloatParameter(RADIUS, GetSize());
	SetAngle(comp, angle);
}

void UObjectiveComponent::BeginPlay()
{
	Super::BeginPlay();
	SetupParticles(&currentTeamParticles, "CurrentTeamParticles", 0);
	SetupParticles(&transitioningParticles, "TransitioningParticles", 360);
	SetSize(GetSize());
	GetGameMode()->AddObjective(this);
	CheckForOverlaps();
	setupComplete = true;
	OnObjectiveParticlesSetup.Broadcast();
}

void UObjectiveComponent::SetSize(float inKeyActorSize)
{
	size = inKeyActorSize;
	objectiveArea->SetSphereRadius(GetSize() * 1.2);

	if (IsValid(currentTeamParticles))
	{
		currentTeamParticles->SetFloatParameter(RADIUS, GetSize());
	}

	if (IsValid(transitioningParticles))
	{
		transitioningParticles->SetFloatParameter(RADIUS, GetSize());
	}
}

void UObjectiveComponent::SetAngle(UNiagaraComponent* comp, float angle)
{
	comp->SetFloatParameter(PERCENT, angle);
	comp->SetActive(angle > 10);
}

FString UObjectiveComponent::GetObjectiveName()
{
	if (objectiveName.IsEmpty())
	{
		if (IsValid(GetGameMode()))
		{
			objectiveName = "Objective " + FString::FromInt(GetGameMode()->GetObjectives().IndexOfByKey(this));
		}
	}
	return objectiveName;
}

void UObjectiveComponent::CalculateOwnership()
{
	// Clear the teams list, as we're calculating it again 
	teamsInArea.Empty();

	for (FCombatantData combatant : combatantsInArea)
	{
		if (!teamsInArea.Contains(combatant.GetTeam()) && combatant.IsAlive())
		{
			teamsInArea.Add(combatant.GetTeam());
		}
	}

	// If there's only 1 team in the area, then they have full claim of it
	if (teamsInArea.Num() == 1 && GetAreaOwner() != teamsInArea[0])
	{
		SetAreaOwner(teamsInArea[0]);
	}
}

int32 UObjectiveComponent::GetCurrentOwningTeam()
{
	return previousAreaOwner == areaOwner ? areaOwner : previousAreaOwner;
}

void UObjectiveComponent::UpdateColour()
{
	// Check if we have exceeded the minimum control value, if so then we can change the colour to the owning team
	// Check if the previousAreaOwner and areaOwner are the same, this means the colour can change as the previousAreaOwner isn't an enemy team
	if (currentControl > 0 && previousAreaOwner == areaOwner)
	{
		FColor teamColour = GetGameMode()->GetTeamColour(areaOwner);

		if (currentColour != teamColour)
		{
			currentColour = teamColour;
			currentTeamParticles->SetColorParameter(COLOUR, FLinearColor(currentColour));
		}
	}
}

void UObjectiveComponent::SetAreaOwner(int32 val)
{
	areaOwner = val;
}

void UObjectiveComponent::CalculateClaim()
{
	float startingCurrentControl = currentControl;

	// If only one team is in the area, then they  can start to claim it
	if (teamsInArea.Num() == 1)
	{
		// If the previousAreaOwner is 0 and there's a new owner then start to claim, this is only ever the case if it's yet to be claimed 
		if (previousAreaOwner == 0 && areaOwner != 0)
		{
			currentControl += combatantsInArea.Num() * scoreMultiplier;
			currentControl = mClampValue<int32>(currentControl, maxControl, 0);

			// Once the control exceeds the minimum control, the new team can have control
			if (currentControl >= minControl)
			{
				previousAreaOwner = areaOwner;
			}
		}
		// If the area owner isn't the same as the last and the area has some control, start to remove the control from the existing team
		else if (previousAreaOwner != areaOwner && currentControl > 0)
		{
			currentControl -= combatantsInArea.Num() * scoreMultiplier;
			currentControl = mClampValue<int32>(currentControl, maxControl, 0);

			// If the control is now 0, then we've removed all existing control and can start to claim it
			if (currentControl == 0)
			{
				previousAreaOwner = areaOwner;
			}
		}
		// If the previousAreaOwner and areaOwner are the same, then that team has control and we can start claiming it
		// Check if we're not at the max control
		else if (previousAreaOwner == areaOwner && currentControl < maxControl)
		{
			currentControl += combatantsInArea.Num();
			currentControl = mClampValue<int32>(currentControl, maxControl, 0);
		}

		// Check if we have full control and we've not already got full claim
		// If we have this level of control, the make the particles bigger
		if (currentControl == maxControl && !fullClaim)
		{
			UE_LOG(LogObjectives, Log, TEXT("Team %d claimed objective %s"), previousAreaOwner, *GetObjectiveName());
			currentTeamParticles->SetFloatParameter(SIZE, bigParticle);
			fullClaim = true;

			if (OnObjectiveClaimed.IsBound())
			{
				OnObjectiveClaimed.Broadcast(this);
			}
		}
		// If the control is less than max then make the particles smaller, this makes it easier to tell when it's fully claimed
		else if (currentControl < maxControl && fullClaim)
		{
			UE_LOG(LogObjectives, Log, TEXT("Team %d is loosing claim to objective %s"), previousAreaOwner, *GetObjectiveName());
			currentTeamParticles->SetFloatParameter(SIZE, smallParticle);
			fullClaim = false;
		}

		if (currentControl != startingCurrentControl)
		{
			UpdateColour();

			float radius = 360 * (currentControl / maxControl);
			float radiusDiff = 360 - radius;

			currentTeamParticles->SetFloatParameter(PERCENT, radius);
			transitioningParticles->SetFloatParameter(PERCENT, radiusDiff);
			SetAngle(currentTeamParticles, radius);
			SetAngle(transitioningParticles, radiusDiff);

			transitioningParticles->SetFloatParameter(ROTATION, (radius / 2) + (radiusDiff / 2));
		}
	}
}

void UObjectiveComponent::Add(FCombatantData combatant)
{
	bool notOwner = IsValid(GetOwner()) && combatant != GetOwner();

	if (notOwner && combatant.IsValid() && combatant.IsAlive())
	{
		//UE_LOG(LogObjectives, Log, TEXT("%s entered objective %s"), *combatant.GetCombatantName(), *GetObjectiveName());
		combatantsInArea.AddUnique(combatant);
		combatant.healthComponent->OnUnitDied.AddUniqueDynamic(this, &UObjectiveComponent::UnitDied);
		CalculateOwnership();
		UE_LOG(LogObjectives, Log, TEXT("%s added combatant %s, total %d"), *GetObjectiveName(), *combatant.GetCombatantName(), combatantsInArea.Num());
	}
}

void UObjectiveComponent::Remove(const FCombatantData& combatant)
{
	if (combatantsInArea.Contains(combatant))
	{
		//UE_LOG(LogObjectives, Log, TEXT("%s left objective %s"), *combatant.GetCombatantName(), *GetObjectiveName());
		combatantsInArea.Remove(combatant);
		combatant.healthComponent->OnUnitDied.RemoveDynamic(this, &UObjectiveComponent::UnitDied);
		CalculateOwnership();
		UE_LOG(LogObjectives, Log, TEXT("%s removed combatant %s, total %d"), *GetObjectiveName(), *combatant.GetCombatantName(), combatantsInArea.Num());
	}
}

ADroneRPGGameMode* UObjectiveComponent::GetGameMode()
{
	if (!IsValid(gameMode))
	{
		gameMode = Cast<ADroneRPGGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	}
	return gameMode;
}

void UObjectiveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (setupComplete && IsValid(currentTeamParticles) && IsValid(transitioningParticles))
	{
		CalculateClaim();

		overlapTimePassed += DeltaTime;

		// Every second add 5 points to the team that full owns this point
		if (fullClaim)
		{
			GetGameMode()->AddTeamScore(areaOwner, 50);
		}

		if (overlapTimePassed > overlapTimeRate)
		{
			overlapTimePassed = 0;
			CheckForOverlaps();
		}
	}
}

bool UObjectiveComponent::HasCompleteControl(int32 team)
{
	return fullClaim && areaOwner == team;
}

float UObjectiveComponent::GetCurrentControlPercent()
{
	return (currentControl / maxControl) * 100;
}
#include "ObjectiveComponent.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include <Components/StaticMeshComponent.h>
#include <DroneRPG/Components/CombatantComponent.h>
#include <Kismet/GameplayStatics.h>
#include "Components/SphereComponent.h"
#include "DroneRPG/Components/HealthComponent.h"
#include "DroneRPG/Utilities/FunctionLibrary.h"
#include "DroneRPG/GameModes/DroneRPGGameMode.h"
#include "DroneRPG/Utilities/CombatClasses.h"

UObjectiveComponent::UObjectiveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1;
	objectiveName = "";

	areaOwner = 0;
	currentControl = 0;
	minControl = 3;
	maxControl = 10;
	fullClaim = false;
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
	UFunctionLibrary::SetupOverlap(objectiveArea);
}

void UObjectiveComponent::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UCombatantComponent* combatant = mGetCombatantComponent(OtherActor);

	// Check if we have a drone and we have it in the list
	if (IsValid(combatant))
	{
		// Add it to the list and re-calculate ownership
		Add(combatant);
	}
}

void UObjectiveComponent::UnitDied(UCombatantComponent* unit)
{
	Remove(unit);
}

void UObjectiveComponent::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UCombatantComponent* combatantComponent = mGetCombatantComponent(OtherActor);

	// Check if we have a drone and we have it in the list
	if (IsValid(combatantComponent))
	{
		// Remove it from the list and re-calculate ownership
		Remove(combatantComponent);
	}
}

void UObjectiveComponent::CheckForOverlaps()
{
	TArray<AActor*> overlaps;
	objectiveArea->GetOverlappingActors(overlaps);

	for (AActor* overlap : overlaps)
	{
		UCombatantComponent* combatant = mGetCombatantComponent(overlap);

		// Check if we have a drone and we have it in the list
		if (IsValid(combatant))
		{
			Add(combatant);
		}
	}
}

void UObjectiveComponent::BeginPlay()
{
	Super::BeginPlay();

	// Bind to the box components begin and end overlap events
	objectiveArea->OnComponentBeginOverlap.AddDynamic(this, &UObjectiveComponent::BeginOverlap);
	objectiveArea->OnComponentEndOverlap.AddDynamic(this, &UObjectiveComponent::EndOverlap);

	currentTeamParticles = UNiagaraFunctionLibrary::SpawnSystemAttached(auraSystem, GetOwner()->GetRootComponent(), TEXT("currentTeamParticles"), FVector(1), FRotator(1), EAttachLocation::SnapToTarget, false);
	transitioningParticles = UNiagaraFunctionLibrary::SpawnSystemAttached(auraSystem, GetOwner()->GetRootComponent(), TEXT("transitioningParticles"), FVector(1), FRotator(1), EAttachLocation::SnapToTarget, false);

	currentTeamParticles->SetFloatParameter(TEXT("Radius"), GetSize());
	currentTeamParticles->SetColorParameter(TEXT("Colour"), FLinearColor(FColor::Red));
	currentTeamParticles->SetFloatParameter(TEXT("Size"), smallParticle);
	SetAngle(currentTeamParticles, 0);

	transitioningParticles->SetFloatParameter(TEXT("Radius"), GetSize());
	transitioningParticles->SetFloatParameter(TEXT("Size"), smallParticle);
	transitioningParticles->SetColorParameter(TEXT("Colour"), FLinearColor(FColor::Red));
	SetAngle(currentTeamParticles, 360);
	CalculateOwnership();

	// Update the colour in here, as we may have started with a team controlling us, set in the editor etc.
	UpdateColour();

	CheckForOverlaps();

	GetGameMode()->AddObjective(this);
	//objectiveArea->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, false));
	objectiveArea->SetSphereRadius(GetSize() * 1.2);
}

void UObjectiveComponent::SetAngle(UNiagaraComponent* comp, float angle)
{
	comp->SetFloatParameter(TEXT("Percent"), angle);
	comp->SetActive(angle > 10);
}

void UObjectiveComponent::CalculateOwnership()
{
	// Clear the teams list, as we're calculating it again 
	teamsInArea.Empty();

	for (UCombatantComponent* component : combatantsInArea)
	{
		UHealthComponent* healthComponent = mGetHealthComponent(component->GetOwner());

		if (!teamsInArea.Contains(component->GetTeam()) && healthComponent->IsAlive())
		{
			teamsInArea.Add(component->GetTeam());
		}
	}

	// If there's only 1 team in the area, then they have full claim of it
	if (teamsInArea.Num() == 1 && GetAreaOwner() != teamsInArea[0])
	{
		SetAreaOwner(teamsInArea[0]);
	}
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
			currentTeamParticles->SetColorParameter(TEXT("Colour"), FLinearColor(currentColour));
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
			currentControl += combatantsInArea.Num();
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
			currentControl -= combatantsInArea.Num();
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
			currentTeamParticles->SetFloatParameter(TEXT("Size"), bigParticle);
			fullClaim = true;

			if (OnObjectiveClaimed.IsBound())
			{
				OnObjectiveClaimed.Broadcast(this);
			}
		}
		// If the control is less than max then make the particles smaller, this makes it easier to tell when it's fully claimed
		else if (currentControl < maxControl && fullClaim)
		{
			currentTeamParticles->SetFloatParameter(TEXT("Size"), smallParticle);
			fullClaim = false;
		}


		if (currentControl != startingCurrentControl)
		{
			UpdateColour();

			float radius = 360 * (currentControl / maxControl);
			float radiusDiff = 360 - radius;

			currentTeamParticles->SetFloatParameter(TEXT("Percent"), radius);
			transitioningParticles->SetFloatParameter(TEXT("Percent"), radiusDiff);
			SetAngle(currentTeamParticles, radius);
			SetAngle(transitioningParticles, radiusDiff);

			transitioningParticles->SetFloatParameter(TEXT("Rotation"), (radius / 2) + (radiusDiff / 2));
		}
	}
}

void UObjectiveComponent::Add(UCombatantComponent* combatant)
{
	UHealthComponent* healthComponent = mGetHealthComponent(combatant->GetOwner());

	if (!combatantsInArea.Contains(combatant) && IsValid(healthComponent) && healthComponent->IsAlive())
	{
		combatantsInArea.AddUnique(combatant);
		healthComponent->OnUnitDied.AddDynamic(this, &UObjectiveComponent::UnitDied);
		CalculateOwnership();
	}
}

void UObjectiveComponent::Remove(UCombatantComponent* combatant)
{
	if (combatantsInArea.Contains(combatant))
	{
		UHealthComponent* healthComponent = mGetHealthComponent(combatant->GetOwner());
		combatantsInArea.Remove(combatant);
		healthComponent->OnUnitDied.RemoveDynamic(this, &UObjectiveComponent::UnitDied);
		CalculateOwnership();
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
	CalculateClaim();

	overlapTimePassed += DeltaTime;

	// Every second add 5 points to the team that full owns this point
	if (fullClaim)
	{
		GetGameMode()->AddTeamScore(areaOwner, 50);
	}
	else if (overlapTimePassed > overlapTimeRate)
	{
		overlapTimePassed = 0;
		CheckForOverlaps();
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



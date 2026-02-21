#include "Turret.h"

#include "Components/CapsuleComponent.h"
#include "DroneRPG/Components/ObjectiveComponent.h"
#include "DroneRPG/Controllers/TurretController.h"
#include "DroneRPG/GameModes/DroneRPGGameMode.h"
#include "DroneRPG/Utilities/WeaponCreator.h"

ATurret::ATurret() : Super()
{
	AIControllerClass = ATurretController::StaticClass();
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1;

	RootComponent = GetCapsuleComponent();

	objectiveComponent = CreateDefaultSubobject<UObjectiveComponent>("ObjectiveComp");
	objectiveComponent->SetSize(1000);
	objectiveComponent->OnObjectiveClaimed.AddUniqueDynamic(this, &ATurret::ObjectiveClaimed);

	//Tags.Add("Objective");

	// Set size for player capsule
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DroneMesh(TEXT("StaticMesh'/Game/TopDownCPP/Models/Drone.Drone'"));

	if (DroneMesh.Succeeded())
	{
		meshComponent->SetStaticMesh(DroneMesh.Object);
		meshComponent->SetupAttachment(RootComponent);
		meshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);
	meshComponent->SetHiddenInGame(false);

	GetCombatantComponent()->SetupCombatantComponent("Turret", EDamagerType::Turret);
}

void ATurret::SetTeam(int32 newTeam)
{
	Super::SetTeam(newTeam);
}

void ATurret::ObjectiveClaimed(UObjectiveComponent* inObjective)
{
	SetTeam(objectiveComponent->GetAreaOwner());
	GetHealthComponent()->FullHeal();
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	meshComponent->SetHiddenInGame(false);
	GetCombatantComponent()->ResetCombatScore();
}

void ATurret::UnitDied(UCombatantComponent* inKiller)
{
	//Super::UnitDied(inKiller);
}

void ATurret::BeginPlay()
{
	Super::BeginPlay();
	int newTeam = 100;
	SetTeam(newTeam);
	objectiveComponent->SetAreaOwner(newTeam);
	objectiveComponent->SetPreviousAreaOwner(newTeam);
	objectiveComponent->SetAngle(objectiveComponent->GetCurrentTeamParticles(), 360);
	objectiveComponent->SetAngle(objectiveComponent->GetTransitioningParticles(), 0);
	objectiveComponent->SetCurrentControl(10);
	objectiveComponent->SetFullClaim(true);
	objectiveComponent->UpdateColour();

#if WITH_EDITOR
	SetFolderPath(TEXT("Turrets"));
#endif

	objectiveComponent->GetCombatantsInArea().Remove(GetCombatantComponent());
}

void ATurret::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	SetWeapon(mGetDefaultWeapon(EWeaponType::Laser, GetCombatantComponent()));
}

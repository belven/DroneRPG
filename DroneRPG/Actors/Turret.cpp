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
	objectiveComponent->OnObjectiveClaimed.AddUniqueDynamic(this, &ATurret::ObjectiveClaimed);
	objectiveComponent->OnObjectiveParticlesSetup.AddUniqueDynamic(this, &ATurret::ObjectiveParticlesSetup);
	objectiveComponent->SetObjectiveName("Turret");
	objectiveComponent->SetScoreMultiplier(3);
	objectiveComponent->SetSize(1000);

	// Set size for player capsule
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DroneMesh(TEXT("StaticMesh'/Game/TopDownCPP/Models/Drone.Drone'"));

	if (DroneMesh.Succeeded())
	{
		meshComponent->SetStaticMesh(DroneMesh.Object);
		meshComponent->SetupAttachment(RootComponent);
		meshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		meshComponent->SetHiddenInGame(false);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);

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

void ATurret::UnitDied(AActor* unitKilled, UCombatantComponent* inKiller)
{
	//Super::UnitDied(inKiller);
}

void ATurret::ObjectiveParticlesSetup()
{
	int newTeam = 100;
	SetTeam(newTeam);
	objectiveComponent->SetTeamClaim(newTeam);
}

void ATurret::BeginPlay()
{
	Super::BeginPlay();

#if WITH_EDITOR
	SetFolderPath(TEXT("Turrets"));
#endif
}

void ATurret::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	SetWeapon(mGetDefaultWeapon(EWeaponType::Laser, GetCombatantComponent()));
}

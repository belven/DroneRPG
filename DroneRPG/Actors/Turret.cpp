#include "Turret.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "DroneRPG/Components/ObjectiveComponent.h"
#include "DroneRPG/Controllers/TurretController.h"
#include "DroneRPG/GameModes/DroneRPGGameMode.h"
#include "DroneRPG/Utilities/WeaponCreator.h"
#include "GameFramework/CharacterMovementComponent.h"

ATurret::ATurret() : Super()
{
	AIControllerClass = ATurretController::StaticClass();
	GetCharacterMovement()->SetMovementMode(MOVE_None);

	objectiveComponent = CreateDefaultSubobject<UObjectiveComponent>("ObjectiveComp");
	objectiveComponent->OnObjectiveClaimed.AddUniqueDynamic(this, &ATurret::ObjectiveClaimed);
	objectiveComponent->OnObjectiveParticlesSetup.AddUniqueDynamic(this, &ATurret::ObjectiveParticlesSetup);
	objectiveComponent->SetObjectiveName("Turret");
	objectiveComponent->SetScoreMultiplier(5);
	objectiveComponent->SetSize(1000);
	objectiveComponent->GetObjectiveArea()->SetupAttachment(GetCapsuleComponent());

	// Set size for player capsule
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DroneMesh(TEXT("StaticMesh'/Game/TopDownCPP/Models/Drone.Drone'"));

	if (DroneMesh.Succeeded())
	{
		meshComponent->SetStaticMesh(DroneMesh.Object);
		meshComponent->SetupAttachment(RootComponent);
		meshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		meshComponent->SetHiddenInGame(false);
	}

	GetCombatantComponent()->SetupCombatantComponent("Turret", EDamagerType::Turret);
}

void ATurret::SetTeam(int32 newTeam)
{
	Super::SetTeam(newTeam);
}

void ATurret::ObjectiveClaimed(UObjectiveComponent* inObjective)
{
	SetTeam(objectiveComponent->GetPreviousAreaOwner());
	GetHealthComponent()->FullHeal();
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	meshComponent->SetHiddenInGame(false);
	GetCombatantComponent()->ResetCombatScore();
}

void ATurret::UnitDied(AActor* unitKilled, UCombatantComponent* inKiller)
{
	//Super::UnitDied(unitKilled, inKiller);
}

void ATurret::ObjectiveParticlesSetup()
{
	SetTeam(100);
	objectiveComponent->SetTeamClaim(GetTeam());
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

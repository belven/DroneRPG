#include "BaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "DroneRPG/Components/CombatantComponent.h"
#include "DroneRPG/Components/HealthComponent.h"
#include "DroneRPG/GameModes/DroneRPGGameMode.h"
#include "DroneRPG/Utilities/FunctionLibrary.h"
#include "DroneRPG/Utilities/WeaponCreator.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set size for player capsule
	const float capWidth = 50;
	const float capHeight = 100;

	GetCapsuleComponent()->InitCapsuleSize(capWidth, capHeight);
	GetCapsuleComponent()->SetCollisionProfileName("Pawn");
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);

	meshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DroneMesh"));

	healthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
	healthComponent->OnUnitDied.AddUniqueDynamic(this, &ABaseCharacter::UnitDied);
	healthComponent->OnUnitHit.AddUniqueDynamic(this, &ABaseCharacter::UnitHit);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 150.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
	GetCharacterMovement()->SetMovementMode(MOVE_NavWalking);
	GetCharacterMovement()->MaxWalkSpeed = 1500;
	combatantComponent = CreateDefaultSubobject<UCombatantComponent>(TEXT("CombatComp"));
}

void ABaseCharacter::UnitDied(UCombatantComponent* inKiller)
{
	GetCombatantComponent()->IncrementDeaths();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);
	meshComponent->SetHiddenInGame(true);

	// Tell the killer they've killed us
	inKiller->UnitKilled(GetCombatantComponent());

	// Tell the gamemode we've died, to update score etc.
	GetGameMode()->EntityKilled(GetCombatantComponent(), inKiller);
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

#if WITH_EDITOR
	SetFolderPath(TEXT("Characters"));
#endif
}

FString ABaseCharacter::GetCharacterName()
{
	if (characterName.IsEmpty() && GetGameMode()->GetCombatants().Contains(GetCombatantComponent()))
	{
		characterName = "Character " + FString::FromInt(GetGameMode()->GetCombatants().IndexOfByKey(GetCombatantComponent()));
	}
	return characterName;
}


void ABaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	combatantComponent->SetupCombatantComponent(GetCharacterName(), EDamagerType::Drone);
	// Give each drone a random weapon
	EWeaponType type = UFunctionLibrary::GetRandomEnum<EWeaponType>(EWeaponType::End);
	SetWeapon(mGetDefaultWeapon(type, GetCombatantComponent()));
}

int32 ABaseCharacter::GetTeam() const
{
	return IsValid(GetCombatantComponent()) ? GetCombatantComponent()->GetTeam() : -1;
}

void ABaseCharacter::SetTeam(int32 val)
{
	combatantComponent->SetTeam(val);
	FColor colour = GetGameMode()->GetTeamColour(GetTeam());
	healthComponent->SetTeamColour(colour);
	combatantComponent->SetTeamColour(colour);
}

ADroneRPGGameMode* ABaseCharacter::GetGameMode()
{
	if (!IsValid(gameMode))
	{
		gameMode = Cast<ADroneRPGGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		combatantComponent->SetGameMode(gameMode);
	}
	return gameMode;
}

void ABaseCharacter::UnitHit(float damage, UCombatantComponent* attacker)
{
	GetGameMode()->UnitHit(damage, attacker);
}
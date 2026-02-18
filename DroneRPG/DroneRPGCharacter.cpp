#pragma once
#include "DroneRPGCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/DecalComponent.h"
#include "Components/HealthComponent.h"
#include "Components/CombatantComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameModes/DroneRPGGameMode.h"
#include "LevelActors/RespawnPoint.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"
#include "Utilities/FunctionLibrary.h"
#include <Kismet/GameplayStatics.h>
#include "Utilities/WeaponCreator.h"

ADroneRPGCharacter::ADroneRPGCharacter()
{
	// Set size for player capsule
	const float capWidth = 50;
	const float capHeight = 100;

	GetCapsuleComponent()->InitCapsuleSize(capWidth, capHeight);
	GetCapsuleComponent()->SetCollisionProfileName("Pawn");
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DroneMesh(TEXT("StaticMesh'/Game/TopDownCPP/Models/Drone.Drone'"));

	meshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DroneMesh"));

	if (DroneMesh.Succeeded())
	{
		meshComponent->SetStaticMesh(DroneMesh.Object);
		meshComponent->SetupAttachment(RootComponent);
		meshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 150.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
	GetCharacterMovement()->SetMovementMode(MOVE_NavWalking);

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 8000;
	CameraBoom->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	TopDownCameraComponent->SetProjectionMode(ECameraProjectionMode::Orthographic);
	TopDownCameraComponent->SetOrthoWidth(6000);

	// Create a decal in the world to show the cursor's location
	CursorToWorld = CreateDefaultSubobject<UDecalComponent>("CursorToWorld");
	CursorToWorld->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UMaterial> DecalMaterialAsset(TEXT("Material'/Game/TopDownCPP/Blueprints/M_Cursor_Decal.M_Cursor_Decal'"));
	if (DecalMaterialAsset.Succeeded())
	{
		CursorToWorld->SetDecalMaterial(DecalMaterialAsset.Object);
	}

	CursorToWorld->DecalSize = FVector(16.0f, 32.0f, 32.0f);
	CursorToWorld->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	GetCharacterMovement()->MaxWalkSpeed = 1500;

	healthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
	healthComponent->OnUnitDied.AddUniqueDynamic(this, &ADroneRPGCharacter::KillDrone);
	healthComponent->OnUnitHit.AddUniqueDynamic(this, &ADroneRPGCharacter::UnitHit);

	combatantComponent = CreateDefaultSubobject<UCombatantComponent>(TEXT("CombatComp"));
}

void ADroneRPGCharacter::BeginDestroy()
{
	Super::BeginDestroy();
}

FColor ADroneRPGCharacter::GetTeamColour() {
	return GetGameMode()->GetTeamColour(GetTeam());
}

void ADroneRPGCharacter::BeginPlay()
{
	Super::BeginPlay();

	float Radius = GetCapsuleComponent()->GetScaledCapsuleRadius();
	float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

#if WITH_EDITOR
	SetFolderPath(TEXT("Characters"));
#endif

	//	GetGameMode()->AddCombatant(GetCombatantComponent());
}

void ADroneRPGCharacter::SetUpDrone()
{
	// Give each drone a random weapon
	combatantComponent->SetupCombatantComponent(GetDroneName(), EDamagerType::Drone);
	EWeaponType type = UFunctionLibrary::GetRandomEnum<EWeaponType>(EWeaponType::End);
	SetWeapon(mGetDefaultWeapon(type, GetCombatantComponent()));
}

void ADroneRPGCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	SetUpDrone();
}

int32 ADroneRPGCharacter::GetTeam() const
{
	return IsValid(GetCombatantComponent()) ? GetCombatantComponent()->GetTeam() : -1;
}

void ADroneRPGCharacter::SetTeam(int32 val)
{
	combatantComponent->SetTeam(val);
	healthComponent->SetTeamColour(GetGameMode()->GetTeamColour(GetTeam()));
}

ADroneRPGGameMode* ADroneRPGCharacter::GetGameMode()
{
	if (!IsValid(gameMode))
	{
		gameMode = Cast<ADroneRPGGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		combatantComponent->SetGameMode(gameMode);
	}
	return gameMode;
}

void ADroneRPGCharacter::Respawn()
{
	// Get our teams respawn point
	ARespawnPoint* respawn = GetRespawnPoint();

	// Did we find a respawn point?
	if (IsValid(respawn))
	{
		// Move us to the respawn point 
		SetActorLocation(respawn->GetSpawnLocation(), false, NULL, ETeleportType::ResetPhysics);

		// Fully Heal the drone
		healthComponent->FullHeal();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetCapsuleComponent()->SetGenerateOverlapEvents(true);
		meshComponent->SetHiddenInGame(false);
	}
}

void ADroneRPGCharacter::UnitHit(float damage, UCombatantComponent* attacker)
{
	GetGameMode()->UnitHit(damage, attacker);
}

ARespawnPoint* ADroneRPGCharacter::GetRespawnPoint()
{
	if (!IsValid(respawnPoint))
	{
		// Get all the respawn points
		TArray<ARespawnPoint*> respawnPoints = mGetActorsInWorld<ARespawnPoint>(GetWorld());

		for (ARespawnPoint* respawnPointFound : respawnPoints)
		{
			// Check if the respawn point belongs to our team 
			if (respawnPointFound->GetTeam() == GetTeam())
			{
				respawnPoint = respawnPointFound;
			}
		}
	}

	return respawnPoint;
}

void ADroneRPGCharacter::KillDrone(UCombatantComponent* killer)
{
	GetCombatantComponent()->IncrementDeaths();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);
	meshComponent->SetHiddenInGame(true);

	mSetTimer(TimerHandle_Kill, &ADroneRPGCharacter::Respawn, 2.5f);

	// Tell the killer they've killed us
	killer->UnitKilled(GetCombatantComponent());

	// Tell the gamemode we've died, to update score etc.
	GetGameMode()->EntityKilled(GetCombatantComponent(), killer);
}

FString ADroneRPGCharacter::GetDroneName()
{
	if (droneName.IsEmpty() && GetGameMode()->GetCombatants().Contains(GetCombatantComponent()))
	{
		droneName = "Drone " + FString::FromInt(GetGameMode()->GetCombatants().IndexOfByKey(GetCombatantComponent()));
	}
	return droneName;
}
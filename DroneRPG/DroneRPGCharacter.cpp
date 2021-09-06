#include "DroneRPGCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include <Kismet/GameplayStatics.h>
#include <Kismet/KismetMathLibrary.h>
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "../Plugins/FX/Niagara/Source/Niagara/Classes/NiagaraSystem.h"
#include "DroneProjectile.h"
#include "DroneBaseAI.h"
#include "FunctionLibrary.h"
#include "RespawnPoint.h"

#define mSetTimer(handle, method, delay) GetWorld()->GetTimerManager().SetTimer(handle, this, &ADroneRPGCharacter::method, delay)

ADroneRPGCharacter::ADroneRPGCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(80.0f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Rotate character to moving direction
	//GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 150.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> auraParticleSystem(TEXT("/Game/TopDownCPP/ParticleEffects/AuraSystem"));

	if (auraParticleSystem.Succeeded()) {
		auraSystem = auraParticleSystem.Object;
	}

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> TrailParticleSystem(TEXT("/Game/TopDownCPP/ParticleEffects/TrailParticleSystem"));

	if (TrailParticleSystem.Succeeded()) {
		trailSystem = TrailParticleSystem.Object;
	}

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 5000.f;
	CameraBoom->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	TopDownCameraComponent->SetProjectionMode(ECameraProjectionMode::Perspective);
	TopDownCameraComponent->SetOrthoWidth(4000);

	// Create a decal in the world to show the cursor's location
	CursorToWorld = CreateDefaultSubobject<UDecalComponent>("CursorToWorld");
	CursorToWorld->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DroneMesh(TEXT("StaticMesh'/Game/TopDownCPP/Models/Drone.Drone'"));

	meshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DroneMesh"));
	if (DroneMesh.Succeeded())
	{
		meshComponent->SetStaticMesh(DroneMesh.Object);
		meshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		meshComponent->SetRelativeLocation(FVector(40, -25, 100));
		meshComponent->SetupAttachment(RootComponent);
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> DecalMaterialAsset(TEXT("Material'/Game/TopDownCPP/Blueprints/M_Cursor_Decal.M_Cursor_Decal'"));
	if (DecalMaterialAsset.Succeeded())
	{
		CursorToWorld->SetDecalMaterial(DecalMaterialAsset.Object);
	}
	CursorToWorld->DecalSize = FVector(16.0f, 32.0f, 32.0f);
	CursorToWorld->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	SetDefaults();

	energyRegen = 10;
	shieldRegen = 10;

	shieldRegenDelay = 3.0f;
	energyRegenDelay = 3.0f;

	SetTeam(1);
}

void ADroneRPGCharacter::SetDefaults() {
	float energy = 150;
	float health = 150;
	float shields = 150;

	maxStats.energy = energy;
	maxStats.health = health;
	maxStats.shields = shields;

	currentStats.energy = energy;
	currentStats.health = health;
	currentStats.shields = shields;

	canRegenShields = true;
	shieldsCritical = false;
	healthStatus = FColor::Green;
	shieldsActive = true;
}

void ADroneRPGCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

float ADroneRPGCharacter::ClampValue(float value, float max, float min) {
	if (value < min)
		value = min;

	if (value > max)
		value = max;

	return value;
}

void ADroneRPGCharacter::Respawn() {
	ARespawnPoint* respawn = GetRespawnPoint();

	if (respawn != NULL) {
		SetDefaults();
		SetActorLocation(respawn->GetActorLocation());

		meshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		meshComponent->SetHiddenInGame(false);

		shieldParticle->Activate();
		healthParticle->Activate();

		healthParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Green));
		shieldParticle->SetFloatParameter(TEXT("Size"), 45);
		shieldParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Cyan));
	}
}

ARespawnPoint* ADroneRPGCharacter::GetRespawnPoint()
{
	TArray<ARespawnPoint*> respawnPoints = mGetActorsInWorld<ARespawnPoint>(GetWorld());

	for (ARespawnPoint* respawnPoint : respawnPoints) {
		if (respawnPoint->GetTeam() == team) {
			return respawnPoint;
		}
	}

	return NULL;
}

void ADroneRPGCharacter::KillDrone() {
	currentStats.health = 0;
	currentStats.shields = 0;
	canRegenShields = false;

	meshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	meshComponent->SetHiddenInGame(true);

	shieldParticle->DeactivateImmediate();
	healthParticle->DeactivateImmediate();

	mSetTimer(TimerHandle_Kill, Respawn, 3.0f);
}

void ADroneRPGCharacter::RecieveHit(ADroneProjectile* projectile) {
	// Fixed damage for now, need to create different projectiles with different damage TODO:
	float damage = 15;

	// Disable our shield regen as we've been hit
	canRegenShields = false;
	mSetTimer(TimerHandle_ShieldRegenRestart, StartShieldRegen, shieldRegenDelay);

	// If we have 0 shields, then take health damage
	if (currentStats.shields <= 0) {
		currentStats.health -= damage;

		// If we have no health, kill the character TODO: set up the concept of respawning the player and making a spectator mode whilst that's happening
		if (currentStats.health <= 0) {
			KillDrone();
		}

		CalculateHealthColours();
	}
	// Otherwise take damage to shields TODO: do we want to calculate excess damage i.e. left over damage goes into health after shields?
	else {
		currentStats.shields -= damage;

		// Weaken max shields, to prevent ships being as strong for the whole match
		if (maxStats.shields > 50) {

			// Take half damage taken away from max shields TODO work out a decent value for this, change it based on projectile?
			maxStats.shields -= (damage * 0.5);
			ClampValue(maxStats.shields, maxStats.shields, 50);
		}
		CalculateShieldParticles();
	}

	// Inform our controller that we've been hit, only the AI version needs to know for now, so it can respond to combat
	if (GetController() != NULL && mIsA(GetController(), ADroneBaseAI)) {
		ADroneBaseAI* con = Cast<ADroneBaseAI>(GetController());
		con->DroneAttacked(projectile->GetShooter());
	}

	ClampValue(currentStats.health, maxStats.health, 0);
	ClampValue(currentStats.shields, maxStats.shields, 0);
}

bool ADroneRPGCharacter::IsAlive()
{
	return currentStats.health > 0;
}

void ADroneRPGCharacter::CalculateHealthColours() {
	// Update the colours, above 70% Green, above 45% Orange and below 45% Red
	if (currentStats.health < (maxStats.health * 0.45f) && healthStatus != FColor::Red) {
		healthParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Red));
		healthStatus = FColor::Red;
	}
	else if (currentStats.health < (maxStats.health * 0.7f) && healthStatus != FColor::Orange && healthStatus != FColor::Red) {
		healthParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Orange));
		healthStatus = FColor::Orange;
	}
	else if (currentStats.health > (maxStats.health * 0.7f) && healthStatus != FColor::Green) {
		healthParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Green));
		healthStatus = FColor::Green;
	}
}

void ADroneRPGCharacter::CalculateShieldParticles() {
	// Change the colour and size of the particles base on shield value, they'll be smaller and darker if we have < 50% shields
	if (currentStats.shields < (maxStats.shields * 0.5f) && !shieldsCritical) {
		shieldParticle->SetFloatParameter(TEXT("Size"), 35);
		shieldParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Blue));
		shieldsCritical = true;
	}
	else if (currentStats.shields > (maxStats.shields * 0.5f) && shieldsCritical) {
		shieldParticle->SetFloatParameter(TEXT("Size"), 45);
		shieldParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Cyan));
		shieldsCritical = false;
	}

	// If we have 0 shields, disable the particle effect
	if (currentStats.shields <= 0 && shieldsActive) {
		shieldParticle->DeactivateImmediate();
		shieldsActive = false;
	}
	else if (currentStats.shields > 0 && !shieldsActive) {
		shieldParticle->ActivateSystem();
		shieldsActive = true;
	}
}

void ADroneRPGCharacter::CalculateShields(float DeltaSeconds) {
	// If our shields aren't at max and we haven't recently been hit, regen our shields
	if (currentStats.shields < maxStats.shields && canRegenShields) {
		float value = shieldRegen * DeltaSeconds;

		// Do we have the energy to regen our shields? TODO: show how represent energy as a particle etc. Do we want to make energy regen stop when used?
		if (currentStats.energy > value) {
			currentStats.shields += value;
			currentStats.energy -= value;

			ClampValue(currentStats.shields, maxStats.shields, 0);
			ClampValue(currentStats.energy, maxStats.energy, 0);
		}
		CalculateShieldParticles();
	}
}

void ADroneRPGCharacter::CalculateEnergy(float DeltaSeconds) {
	// If our current energy isn't at max, restore some energy
	if (currentStats.energy < maxStats.energy) {
		float value = energyRegen * DeltaSeconds;
		currentStats.energy += value;

		ClampValue(currentStats.energy, maxStats.energy, 0);
	}
}

void ADroneRPGCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CalculateEnergy(DeltaSeconds);
	CalculateShields(DeltaSeconds);
}

void ADroneRPGCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Set up particle effect defaults
	shieldParticle = UNiagaraFunctionLibrary::SpawnSystemAttached(auraSystem, RootComponent, TEXT("shieldParticle"), FVector(1), FRotator(1), EAttachLocation::SnapToTarget, false);
	healthParticle = UNiagaraFunctionLibrary::SpawnSystemAttached(auraSystem, RootComponent, TEXT("healthParticle"), FVector(1), FRotator(1), EAttachLocation::SnapToTarget, false);

	shieldParticle->SetFloatParameter(TEXT("Radius"), 125);
	healthParticle->SetFloatParameter(TEXT("Radius"), 125);

	shieldParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Cyan));
	healthParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Green));

	shieldParticle->SetFloatParameter(TEXT("Size"), 45);
	healthParticle->SetFloatParameter(TEXT("Size"), 20);

	// TODO: figure out why this parameter isn't being used correctly! This makes the particles only appear on the top of the sphere
	//shieldParticle->SetBoolParameter(TEXT("Hem Z"), true);
	//healthParticle->SetBoolParameter(TEXT("Hem Z"), true);
}

void ADroneRPGCharacter::StartShieldRegen()
{
	canRegenShields = true;
}


// Copyright Epic Games, Inc. All Rights Reserved.

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
#include <DroneBaseAI.h>

#define MIN(a,b) (a < b) ? (a) : (b)
#define MAX(a,b) (a > b) ? (a) : (b)
#define CLAMP(value, max, min) (value = (MAX(MIN(value, max), min)))
#define mSetTimer(handle, method, delay) GetWorld()->GetTimerManager().SetTimer(handle, this, &ADroneRPGCharacter::method, delay)
#define  mIsA(aObject, aClass)  aObject->IsA(aClass::StaticClass())

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
	CameraBoom->TargetArmLength = 1400.f;
	CameraBoom->SetRelativeRotation(FRotator(-75.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

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


	float energy = 150;
	float health = 150;
	float shields = 150;

	maxStats.energy = energy;
	maxStats.health = health;
	maxStats.shields = shields;

	currentStats.energy = energy;
	currentStats.health = health;
	currentStats.shields = shields;

	energyRegen = 10;
	shieldRegen = 10;

	shieldRegenDelay = 3.0f;
	energyRegenDelay = 3.0f;

	canRegenShields = true;
	shieldsCritical = false;
	healthStatus = FColor::Green;
	shieldsActive = true;

	SetTeam(1);
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

void ADroneRPGCharacter::RecieveHit(ADroneProjectile* projectile) {
	float damage = 15;

	canRegenShields = false;
	mSetTimer(TimerHandle_ShieldRegenRestart, StartShieldRegen, shieldRegenDelay);

	if (currentStats.shields <= 0) {
		currentStats.health -= damage;

		if (currentStats.health <= 0)
			Destroy();

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
	else {
		currentStats.shields -= damage;
		CalculateShieldParticles();
	}

	ADroneBaseAI* con = Cast<ADroneBaseAI>(GetController());

	if (con != NULL) {
		con->DroneAttacked(projectile->GetShooter());
	}

	ClampValue(currentStats.health, maxStats.health, 0);
	ClampValue(currentStats.shields, maxStats.shields, 0);
}

void ADroneRPGCharacter::CalculateShieldParticles() {
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
	if (currentStats.shields < maxStats.shields && canRegenShields) {
		float value = shieldRegen * DeltaSeconds;

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

	if (engineParticle != NULL) {
		if (GetCharacterMovement()->IsMovementInProgress()) {
			engineParticle->ActivateSystem();
		}
		else {
			engineParticle->DeactivateImmediate();
		}
	}
}

void ADroneRPGCharacter::BeginPlay()
{
	Super::BeginPlay();

	engineParticle = UNiagaraFunctionLibrary::SpawnSystemAttached(trailSystem, RootComponent, TEXT("engineParticle"), FVector(1), FRotator(1), EAttachLocation::SnapToTarget, false);
	shieldParticle = UNiagaraFunctionLibrary::SpawnSystemAttached(auraSystem, RootComponent, TEXT("shieldParticle"), FVector(1), FRotator(1), EAttachLocation::SnapToTarget, false);
	healthParticle = UNiagaraFunctionLibrary::SpawnSystemAttached(auraSystem, RootComponent, TEXT("healthParticle"), FVector(1), FRotator(1), EAttachLocation::SnapToTarget, false);

	shieldParticle->SetFloatParameter(TEXT("Radius"), 125);
	healthParticle->SetFloatParameter(TEXT("Radius"), 125);

	shieldParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Cyan));
	healthParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Green));

	shieldParticle->SetFloatParameter(TEXT("Size"), 45);
	healthParticle->SetFloatParameter(TEXT("Size"), 20);

	if (mIsA(GetController(), ADroneBaseAI))
		SetTeam(2);

	//shieldParticle->SetBoolParameter(TEXT("Hem Z"), true);
	//healthParticle->SetBoolParameter(TEXT("Hem Z"), true);
}

void ADroneRPGCharacter::StartShieldRegen()
{
	canRegenShields = true;
}

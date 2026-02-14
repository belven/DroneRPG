#pragma once
#include "HealthComponent.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "DroneRPG/Utilities/FunctionLibrary.h"
#include "Materials/MaterialInstanceConstant.h"

#define mSpawnSystemAttached(system, name) UNiagaraFunctionLibrary::SpawnSystemAttached(system, GetOwner()->GetRootComponent(), name, FVector(1), FRotator(1), EAttachLocation::SnapToTarget, false)

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetDefaults();

	energyRegen = 10;
	shieldRegen = 10;

	shieldRegenDelay = 3.0f;
	energyRegenDelay = 3.0f;

	smallShieldExp = 40;
	largeShieldExp = 20;
	maxWipe = 0.2;
	minWipe = -0.2;
	wipeValue = FMath::RandRange(minWipe, maxWipe);

	healthParticleSize = 50;

	static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> shieldInstance(TEXT("/ Script/Engine.MaterialInstanceConstant'/Game/TopDownCPP/Materials/Shield_Inst.Shield_Inst'"));

	if (shieldInstance.Succeeded()) {
		matInstanceConst = shieldInstance.Object;
	}

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> auraParticleSystem(TEXT("/Game/TopDownCPP/ParticleEffects/AuraSystem"));

	if (auraParticleSystem.Succeeded()) {
		auraSystem = auraParticleSystem.Object;
	}
}

void UHealthComponent::DamageShields(float& damage)
{
	// We have less shields than damage, so remove are shields from the damage, to allow us to take it as health
	if (currentStats.shields < damage)
	{
		currentStats.shields = 0;
	}
	// We have more shields than damage dealt, so take it all to shields
	else
	{
		currentStats.shields -= damage;
	}

	// Weaken max shields, to prevent ships being as strong for the whole match
	if (maxStats.shields > 50)
	{
		// Take half damage taken away from max shields
		maxStats.shields -= (damage * 0.5);
		UFunctionLibrary::ClampValue(maxStats.shields, maxStats.shields, 50.f);
	}

	damage -= currentStats.shields;
}

void UHealthComponent::ReceiveDamage(float damage, UCombatantComponent* damager)
{
	OnUnitHit.Broadcast(damage, damager);

	// Disable our shield regen as we've been hit
	canRegenShields = false;
	mSetTimer(TimerHandle_ShieldRegenRestart, &UHealthComponent::StartShieldRegen, shieldRegenDelay);

	// Take damage to shields
	if (HasShields())
	{
		DamageShields(damage);
	}

	// Take any remaining damage as health damage
	if (IsAlive() && damage > 0)
	{
		currentStats.health -= damage;

		if (!IsAlive())
		{
			OnUnitDied.Broadcast(damager);
			shieldMeshComp->SetHiddenInGame(true);
			healthParticle->SetHiddenInGame(true);
			shieldsActive = false;
			canRegenShields = false;
		}
	}

	CalculateShieldParticles();
	CalculateHealthColours();

	UFunctionLibrary::ClampValue(currentStats.health, maxStats.health, 0.0f);
	UFunctionLibrary::ClampValue(currentStats.shields, maxStats.shields, 0.0f);
}

void UHealthComponent::SetDefaults()
{
	float energy = 150;
	float health = 150;
	float shields = 250;

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
	canRegenShields = true;
}

bool UHealthComponent::IsAlive()
{
	return currentStats.health > 0;
}

bool UHealthComponent::IsHealthy()
{
	return healthStatus == FColor::Green;
}

void UHealthComponent::FullHeal()
{
	// Set everything back to full
	SetDefaults();

	// Reset the health colour
	healthParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Green));

	//Reset the Exponent of the shield
	SetMaterialFloat(TEXT("Exp"), largeShieldExp);

	// Unhide the health and shield particles
	shieldMeshComp->SetHiddenInGame(false);
	healthParticle->SetHiddenInGame(false);
}

bool UHealthComponent::HasShields()
{
	return currentStats.shields > 0;
}

// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// Set up particle effect defaults
	healthParticle = mSpawnSystemAttached(auraSystem, TEXT("healthParticle"));
	healthParticle->SetFloatParameter(TEXT("Radius"), 225);
	healthParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Green));
	healthParticle->SetFloatParameter(TEXT("Size"), healthParticleSize);

	shieldMesh = LoadObject<UStaticMesh>(this, TEXT("StaticMesh'/Game/TopDownCPP/Models/Shield.Shield'"));

	if (shieldMesh != NULL) 
	{
		// Create our shields, we used an instanced static mesh so the colours can change separately from other drones
		// Otherwise we access the base mesh, that all drones use, and change it globally
		shieldMeshComp = NewObject<UInstancedStaticMeshComponent>(this);
		shieldMeshComp->SetWorldScale3D(FVector(1));
		shieldMeshComp->SetStaticMesh(shieldMesh);
		shieldMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		shieldMeshComp->SetupAttachment(GetOwner()->GetRootComponent());
		shieldMeshComp->RegisterComponent();

		FTransform trans = GetOwner()->GetActorTransform();
		trans.AddToTranslation(FVector(0, 0, 30));
		meshIndex = shieldMeshComp->AddInstance(trans, true);

		// Set default shield values
		SetMaterialFloat(TEXT("Wipe"), minWipe);
		SetMaterialFloat(TEXT("Exp"), largeShieldExp);
	}
}

void UHealthComponent::SetTeamColour(FColor colour)
{
	teamColour = colour;

	FLinearColor col2 = FLinearColor(colour);
	col2.R *= 300;
	col2.G *= 300;
	col2.B *= 300;
	col2.A = 1;
	SetMaterialColour(TEXT("Emissive Color"), col2);
}

void UHealthComponent::CalculateHealthColours()
{
	// Update the colours, above 70% Green, above 45% Orange and below 45% Red
	if (currentStats.health < (maxStats.health * 0.45f) && healthStatus != FColor::Red) 
	{
		healthParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Red));
		healthStatus = FColor::Red;
	}
	else if (currentStats.health < (maxStats.health * 0.7f) && healthStatus != FColor::Orange && healthStatus != FColor::Red) 
	{
		healthParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Orange));
		healthStatus = FColor::Orange;
	}
	else if (currentStats.health > (maxStats.health * 0.7f) && healthStatus != FColor::Green) 
	{
		healthParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Green));
		healthStatus = FColor::Green;
	}
}

void UHealthComponent::CalculateShieldParticles()
{
	// Change the colour and size of the particles base on shield value, they'll be smaller and darker if we have < 50% shields
	if (currentStats.shields < (maxStats.shields * 0.5f) && !shieldsCritical)
	{
		SetMaterialFloat(TEXT("Exp"), smallShieldExp);
		shieldsCritical = true;
	}
	else if (currentStats.shields > (maxStats.shields * 0.5f) && shieldsCritical)
	{
		SetMaterialFloat(TEXT("Exp"), largeShieldExp);
		shieldsCritical = false;
	}

	// If we have 0 shields, disable the particle effect
	if (currentStats.shields <= 0 && shieldsActive)
	{
		shieldMeshComp->SetHiddenInGame(true);
		shieldsActive = false;
	}
	else if (currentStats.shields > 0 && !shieldsActive)
	{
		shieldMeshComp->SetHiddenInGame(false);
		shieldsActive = true;
	}
}

void UHealthComponent::SetMaterialColour(FName param, FLinearColor value)
{
	shieldMeshComp->SetVectorParameterValueOnMaterials(param, FVector(value.R, value.G, value.B));
}

void UHealthComponent::SetMaterialFloat(FName param, float value)
{
	shieldMeshComp->SetScalarParameterValueOnMaterials(param, value);
}

void UHealthComponent::CalculateShields(float DeltaSeconds)
{
	// If our shields aren't at max and we haven't recently been hit, regen our shields
	if (currentStats.shields < maxStats.shields && canRegenShields)
	{
		float value = shieldRegen * DeltaSeconds;

		// Do we have the energy to regen our shields?
		if (currentStats.energy > value)
		{
			currentStats.shields += value;
			currentStats.energy -= value;

			UFunctionLibrary::ClampValue(currentStats.shields, maxStats.shields, 0.f);
			UFunctionLibrary::ClampValue(currentStats.energy, maxStats.energy, 0.f);
		}

		CalculateShieldParticles();
	}
}

void UHealthComponent::CalculateEnergy(float DeltaSeconds)
{
	// If our current energy isn't at max, restore some energy
	if (currentStats.energy < maxStats.energy)
	{
		float value = energyRegen * DeltaSeconds;
		currentStats.energy += value;

		UFunctionLibrary::ClampValue(currentStats.energy, maxStats.energy, 0.f);
	}
}

void UHealthComponent::StartShieldRegen()
{
	canRegenShields = true;
}

void UHealthComponent::PulseShield()
{
	constexpr float increment = 0.005;

	// wipeValue is used to make the shield colour move vertically across the mesh
	wipeValue += increment;

	// If we've reached the top, reset to the bottom
	if (wipeValue >= maxWipe)
	{
		wipeValue = minWipe;
	}

	SetMaterialFloat(TEXT("Wipe"), wipeValue);
}

// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update shield material
	PulseShield();

	CalculateEnergy(DeltaTime);
	CalculateShields(DeltaTime);
}
#include "Rocket.h"
#include <DroneRPG/Components/CombatantComponent.h>
#include "GameFramework/ProjectileMovementComponent.h"
#include "../Plugins/FX/Niagara/Source/Niagara/Classes/NiagaraSystem.h"
#include "Components/SphereComponent.h"
#include "DroneRPG/DroneRPG.h"
#include "DroneRPG/Components/HealthComponent.h"

const float ARocket::Default_Initial_Speed = 2500.0f;
const float ARocket::Default_Initial_Lifespan = 4.5f;

ARocket::ARocket()
{
	ProjectileMovement->InitialSpeed = Default_Initial_Speed;
	ProjectileMovement->MaxSpeed = 3500.0f;
	ProjectileMovement->HomingAccelerationMagnitude = 15000;
	InitialLifeSpan = Default_Initial_Lifespan;

	static ConstructorHelpers::FObjectFinder<USoundBase> FireSoundAudio(TEXT("SoundCue'/Game/TopDownCPP/Sounds/Rocket_Booster_Cue.Rocket_Booster_Cue'"));
	FireSound = FireSoundAudio.Object;

	static ConstructorHelpers::FObjectFinder<USoundBase>HitSoundAudio(TEXT("SoundCue'/Game/TopDownCPP/Sounds/Rocket_Explosion_Cue.Rocket_Explosion_Cue'"));
	HitSound = HitSoundAudio.Object;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> rocketTrailSystem(TEXT("NiagaraSystem'/Game/TopDownCPP/ParticleEffects/Rocket_Trail.Rocket_Trail'"));

	if (rocketTrailSystem.Succeeded())
	{
		trailSystem = rocketTrailSystem.Object;
	}

	sphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RocketOverlap"));
	sphereComponent->SetSphereRadius(500);
	sphereComponent->SetupAttachment(GetRootComponent());
	sphereComponent->SetCollisionProfileName("Projectile");
	sphereComponent->SetGenerateOverlapEvents(true);
	sphereComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &ARocket::BeginOverlap);
}

bool ARocket::SetTargetIfValid(const FCombatantData& targetData)
{
	bool result = false;
	if (CheckIfValidTarget(targetData))
	{
		SetTarget(targetData);
		result = true;
	}
	return result;
}

void ARocket::TargetDied(UCombatantComponent* inKiller)
{
	TargetOverlappingActors();

	if(!target.isSet)
	{
		// TODO create explosion effect for this and DealDamage
		Destroy();
	}
}

void ARocket::SetTarget(FCombatantData targetData)
{
	Super::SetTarget(targetData);

	if (targetData.isSet)
	{
		if (GetShooter() == targetData.combatantComponent)
		{
			UE_LOG(LogDroneAI, Log, TEXT("%s's targeted itself"), *GetShooter()->GetCombatantName());
		}

		ProjectileMovement->HomingTargetComponent = target.combatantComponent->GetOwner()->GetRootComponent();
		ProjectileMovement->bIsHomingProjectile = true;
		targetData.healthComponent->OnUnitDied.AddUniqueDynamic(this, &ARocket::TargetDied);
	}
	else
	{
		ProjectileMovement->bIsHomingProjectile = false;
		ProjectileMovement->HomingTargetComponent = nullptr;
	}
}

void ARocket::DealDamage()
{
	TArray<AActor*> overlaps;
	sphereComponent->GetOverlappingActors(overlaps);

	for (auto overlap : overlaps)
	{
		FCombatantData targetData = mCreateCombatantData(overlap);
		if (CheckIfValidTarget(targetData))
		{
			targetData.healthComponent->ReceiveDamage(GetDamage(), GetShooter());
		}
	}
}

void ARocket::TargetOverlappingActors()
{
	if (IsValid(GetShooter()) && GetShooter()->GetTeam() != -1 && !target.isSet)
	{
		TArray<AActor*> overlaps;
		sphereComponent->GetOverlappingActors(overlaps);

		for (auto overlap : overlaps)
		{
			if (SetTargetIfValid(mCreateCombatantData(overlap)))
			{
				break;
			}
		}
	}
}

void ARocket::SetShooter(UCombatantComponent* val)
{
	Super::SetShooter(val);

	TargetOverlappingActors();
}

void ARocket::HItValidTarget(const FCombatantData& targetData)
{
	DealDamage();
}

void ARocket::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsValid(GetShooter()) && GetShooter()->GetTeam() != -1 && !target.isSet)
	{
		SetTargetIfValid(mCreateCombatantData(OtherActor));
	}
}
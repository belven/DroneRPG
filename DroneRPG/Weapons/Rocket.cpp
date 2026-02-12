#include "Rocket.h"
#include <DroneRPG/Components/CombatantComponent.h>
#include "GameFramework/ProjectileMovementComponent.h"
#include "../Plugins/FX/Niagara/Source/Niagara/Classes/NiagaraSystem.h"
#include "Components/SphereComponent.h"
#include "DroneRPG/Components/HealthComponent.h"
#include "DroneRPG/Utilities/FunctionLibrary.h"

const float ARocket::Default_Initial_Speed = 2500.0f;
const float ARocket::Default_Initial_Lifespan = 4.5f;

ARocket::ARocket()
{
	team = -1;
	ProjectileMovement->InitialSpeed = Default_Initial_Speed;
	ProjectileMovement->MaxSpeed = 3500.0f;
	ProjectileMovement->HomingAccelerationMagnitude = 15000;
	ProjectileMovement->bIsHomingProjectile = true;
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
	sphereComponent->SetSphereRadius(2000);
	sphereComponent->SetupAttachment(GetRootComponent());
	sphereComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
	sphereComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &ARocket::BeginOverlap);
}

bool ARocket::CheckIfValidTarget(const FTargetData& targetData)
{
	return IsValid(targetData.healthComponent) && targetData.healthComponent->IsAlive() && targetData.combatantComponent->GetTeam() != team;
}

bool ARocket::CheckActorForValidTarget(const FTargetData& targetData)
{
	bool result = false;
	if (CheckIfValidTarget(targetData))
	{
		SetTarget(targetData);
		result = true;
	}
	return result;
}

void ARocket::SetTeam(int32 inTeam)
{
	team = inTeam;

	TArray<AActor*> overlaps;
	sphereComponent->GetOverlappingActors(overlaps);

	for (auto overlap : overlaps)
	{
		FTargetData data = CreateTargetData(overlap);
		if (CheckActorForValidTarget(data))
		{
			break;
		}
	}
}

void ARocket::SetTarget(FTargetData targetData)
{
	Super::SetTarget(targetData);
	ProjectileMovement->HomingTargetComponent = target.combatantComponent->GetOwner()->GetRootComponent();
}

void ARocket::DealDamage()
{
	TArray<AActor*> overlaps;
	sphereComponent->GetOverlappingActors(overlaps);

	for (auto overlap : overlaps)
	{
		FTargetData targetData = CreateTargetData(overlap);
		if (CheckIfValidTarget(targetData))
		{
			targetData.healthComponent->ReceiveDamage(GetDamage(), this);
		}
	}
}

void ARocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if (OtherActor != NULL && OtherActor != this && OtherActor != shooter->GetOwner() && !mIsA(OtherActor, ADroneProjectile))
	{
		DealDamage();
		Destroy();
	}
	else
	{
		Destroy();
	}
}

void ARocket::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (team != -1 && !target.isSet)
	{
		CheckActorForValidTarget(CreateTargetData(OtherActor));
	}
}
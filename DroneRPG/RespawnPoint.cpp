// Fill out your copyright notice in the Description page of Project Settings.


#include "RespawnPoint.h"
#include "DroneRPGCharacter.h"
#include "Components/BoxComponent.h"
#include "FunctionLibrary.h"

// Sets default values
ARespawnPoint::ARespawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1;

	respawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("RespawnArea"));
	respawnArea->SetBoxExtent(FVector(1000, 1000, 400));
	respawnArea->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void ARespawnPoint::BeginPlay()
{
	Super::BeginPlay();
	respawnArea->OnComponentBeginOverlap.AddDynamic(this, &ARespawnPoint::BeginOverlap);
	respawnArea->OnComponentEndOverlap.AddDynamic(this, &ARespawnPoint::EndOverlap);
}

void ARespawnPoint::BeginOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (mIsA(OtherActor, ADroneRPGCharacter)) {
		ADroneRPGCharacter* drone = Cast<ADroneRPGCharacter>(OtherActor);
		if (!drone->IsHealthy()) {
			drone->FullHeal();
		}
	}
}

void ARespawnPoint::EndOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{

}

void ARespawnPoint::RespawnCharacter(ADroneRPGCharacter* character) {

}

void ARespawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
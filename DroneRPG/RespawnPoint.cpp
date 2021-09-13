// Fill out your copyright notice in the Description page of Project Settings.


#include "RespawnPoint.h"
#include "DroneRPGCharacter.h"
#include "Components/BoxComponent.h"
#include "FunctionLibrary.h"

// Sets default values
ARespawnPoint::ARespawnPoint()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	respawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("RespawnArea"));
	respawnArea->SetBoxExtent(FVector(700, 700, 400));
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

// Called every frame
void ARespawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
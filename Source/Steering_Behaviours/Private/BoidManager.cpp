// Fill out your copyright notice in the Description page of Project Settings.


#include "BoidManager.h"
#include "Boids.h"

// Sets default values
ABoidManager::ABoidManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	transform = CreateDefaultSubobject<USceneComponent>("Root Scene Component");
	this->SetRootComponent(transform);

}

// Called when the game starts or when spawned
void ABoidManager::BeginPlay()
{
	Super::BeginPlay();
	
	for (int i = 0; i < SpawnCount; i++) 
	{
		FVector SpawnLocation = (FMath::VRand() * FMath::RandRange(0.0f, SpawnRadius)) + GetActorLocation();
		FRotator SpawnRotation = GetActorRotation();

		ABoids* newboid = GetWorld()->SpawnActor<ABoids>(SpawnLocation, SpawnRotation);
		MyBoids.Add(newboid);
	}
}

// Called every frame
void ABoidManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	for (ABoids* Boid : MyBoids)
	{
		Boid->UpdateBoid(DeltaTime);
	}
}

TArray<class ABoids*> ABoidManager::GetBoidNeighbourHood(ABoids* thisBoid) 
{

	TArray<class ABoids*> ReturnBoids;

	for (ABoids* Boid : MyBoids) 
	{
		if (Boid == thisBoid || !Boid) 
		{
			continue;
		}

		float aDistance = (Boid->GetActorLocation() - thisBoid->GetActorLocation()).Size();
		if (aDistance < NeighbourRadius) 
		{
			ReturnBoids.Add(Boid);
		}

	}

	return ReturnBoids;
}




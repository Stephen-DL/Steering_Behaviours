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
	ComputeAvoidanceRays();

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
		newboid->Manager = this;
	}
}

void ABoidManager::ComputeAvoidanceRays()
{
	float GoldenRatio = (1 + FMath::Sqrt(5.0)) / 2;
	float AngleIncrement = PI * 2 * GoldenRatio;

	for (int x = 0; x < NumAvoidancePoints; x++)
	{
		float t = (float)x / NumAvoidancePoints;
		float inclination = FMath::Acos(1 - 2 * t);
		float azimuth = AngleIncrement * x;

		float X = FMath::Sin(inclination) * FMath::Cos(azimuth);
		float Y = FMath::Sin(inclination) * FMath::Sin(azimuth);
		float Z = FMath::Cos(inclination);
		FRotator Rotator = FRotator(-90.0, 0, 0);

		AvoidanceRays.Add(Rotator.RotateVector(FVector(X, Y, Z)));
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

TArray<ABoids*> ABoidManager::GetBoidNeighbourHood(ABoids* thisBoid) 
{



	TArray<ABoids*> ReturnBoids;
	
	if (!thisBoid) return ReturnBoids;



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

	return MyBoids;
}




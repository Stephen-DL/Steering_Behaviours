// Fill out your copyright notice in the Description page of Project Settings.


#include "Boids.h"
#include "BoidManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ABoids::ABoids()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sphere"));
	UStaticMesh* sphereMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'")).Object;

	Mesh->SetStaticMesh(sphereMesh);
	this->SetRootComponent(Mesh);
}

// Called when the game starts or when spawned
void ABoids::BeginPlay()
{
	Super::BeginPlay();
	
}

FVector ABoids::Seek(FVector position)
{
	FVector newVelocity = position - GetActorLocation();
	newVelocity.Normalize();
	return newVelocity;
}

FVector ABoids::Flee(FVector position)
{
	FVector newVelocity = GetActorLocation() - position;
	newVelocity.Normalize();
	return newVelocity;
}

FVector ABoids::Wander(float Radius, float Distance, float Jitter)
{
	FVector myLocation = GetActorLocation();
	if (FVector::Dist(myLocation, WanderDestination) < 100.0f) 
	{
		FVector projectedPos = myLocation + (GetActorForwardVector() * Distance);

		WanderDestination = projectedPos + (FMath::VRand() * FMath::RandRange(0.0f, Jitter));
	}

	FVector jitterLocation = Seek(WanderDestination) + (FMath::VRand() * FMath::RandRange(0.0f, Jitter));
	return jitterLocation;
}

FVector ABoids::Alignment(TArray<ABoids*> Neighbours)
{
	if (Neighbours.Num() == 0) 
	{
		return FVector::ZeroVector;
	}

	FVector newVelocity;
	for (ABoids* Boid : Neighbours) 
	{
		newVelocity += Boid->currentVelocity;
	}

	newVelocity /= Neighbours.Num();
	newVelocity.Normalize();

	return newVelocity;
	
}

FVector ABoids::Cohesion(TArray<ABoids*> Neighbours)
{
	if (Neighbours.Num() == 0)
	{
		return FVector::ZeroVector;
	}

	FVector avgLocation;

	float inverseVal = 1 / Neighbours.Num();

	for (ABoids* Boid : Neighbours)
	{
		avgLocation += (Boid->GetActorLocation() *inverseVal);
	}

	return Seek(avgLocation);
}

FVector ABoids::Seperation(TArray<ABoids*> Neighbours)
{
	if (Neighbours.Num() == 0)
	{
		return FVector::ZeroVector;
	}

	FVector avgFlee;

	for (ABoids* Boid : Neighbours)
	{
		avgFlee += Flee(Boid->GetActorLocation());
	}

	avgFlee.Normalize();
	return avgFlee;
}

// Called every frame
void ABoids::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Manager->DebugAvoidanceInfo)
	{
		DebugAvoidance();
	}
}

void ABoids::UpdateBoid(float DeltaTime)
{
	FVector targetVelocity = FVector::ZeroVector;
	if (waitCounter > 0) 
	{
		waitCounter -= DeltaTime;
	}

	//find velocity
	
	TArray<ABoids*> closestBoids = Manager->GetBoidNeighbourHood(this);

	targetVelocity += Seperation(closestBoids) * Manager->SeperationWeight;
	targetVelocity += Cohesion(closestBoids) * Manager->CohesionWeight;
	targetVelocity += Alignment(closestBoids) * Manager->AlignmentWeight;

	targetVelocity.Normalize();

	if (targetVelocity.Size() < 1.0f) 
	{
		targetVelocity += Wander(100.0f, 200.0f, 50.0f);
		targetVelocity.Normalize();
	}

	FVector newForce = targetVelocity - currentVelocity;
	currentVelocity += newForce * DeltaTime;

	FVector location = GetActorLocation();
	location += (currentVelocity * Manager->Speed * DeltaTime);

	SetActorLocation(location);
}

void ABoids::DebugAvoidance()
{
	TArray<FVector> Rays = Manager->GetAvoidanceRays();

	for (int x = 0; x < Rays.Num(); ++x)
	{
		float color = 255.0 * static_cast<float>(x) / static_cast<float>(Rays.Num());

		FVector WorldSpaceRay = GetTransform().GetRotation().RotateVector(Rays[x]);
		float Angle = FMath::Acos(FVector::DotProduct(WorldSpaceRay, GetActorForwardVector()));

		if (FMath::RadiansToDegrees(Angle) < Manager->ViewAngle)
		{
			DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + WorldSpaceRay * Manager->AvoidanceViewDistance, FColor::Green);
		}
		else
		{
			DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + WorldSpaceRay * Manager->AvoidanceViewDistance, FColor::Red);
		}
	}
}

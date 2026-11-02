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
	Velocity = GetActorForwardVector();

	if (Manager)
	{
		Velocity *= Manager->MaxSpeed;
	}
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

FVector ABoids::CalculateObstacleAvoidance()
{
	// avoidance rays are at increasing angles from the boid forward direction.
	// the idea is to iterate until a raycast doesn't intersect with an obstacle, and then steer in that direction.
	TArray<FVector> Rays = Manager->GetAvoidanceRays();

	FHitResult HitResult;
	float FurthestUnobstructedDistance = 0.0f;
	float ClosestObstructedDistance = std::numeric_limits<float>::max();
	float CloseProximityMultiplier = 1.0;
	FVector BestDirection = GetActorForwardVector();

	ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_GameTraceChannel1);
	EDrawDebugTrace::Type DebugTraceType = EDrawDebugTrace::Type::None;
	TArray<AActor*> ActorsToIgnore;

	for (int x = 0; x < Rays.Num(); ++x)
	{
		// match rotation of ray to boid rotation
		FVector WorldSpaceRay = GetTransform().GetRotation().RotateVector(Rays[x]);

		float Angle = FMath::Acos(FVector::DotProduct(WorldSpaceRay, GetActorForwardVector()));

		if (FMath::RadiansToDegrees(Angle) < Manager->ViewAngle)
		{
			bool TraceHit = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), GetActorLocation(), GetActorLocation() + WorldSpaceRay * Manager->AvoidanceViewDistance,
				50.0, TraceChannel, true, ActorsToIgnore, DebugTraceType, HitResult, true);

			if (TraceHit)
			{
				if (Manager->DebugAvoidanceInfo)
				{
					DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + WorldSpaceRay * HitResult.Distance, FColor::Yellow, false, 1.0, 2.0);
				}

				// if an obstacle is too close, the avoidance force should be stronger
				if (HitResult.Distance < Manager->CloseProximityObstacleRadius)
				{
					if (Manager->DebugAvoidanceInfo)
					{
						DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + WorldSpaceRay * HitResult.Distance, FColor::Red, false, 1.0, 2.0);
					}

					CloseProximityMultiplier = Manager->CloseProximityObstacleAvoidanceFactor;
				}

				if (HitResult.Distance > FurthestUnobstructedDistance)
				{
					BestDirection = WorldSpaceRay;
					FurthestUnobstructedDistance = HitResult.Distance;
				}
			}
			else
			{
				if (Manager->DebugAvoidanceInfo)
				{
					DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + WorldSpaceRay * Manager->AvoidanceViewDistance, FColor::Green, false, 1.0, 2.0);
				}

				return WorldSpaceRay * Manager->MaxSpeed * Manager->ObstacleAvoidanceFactor * CloseProximityMultiplier;
			}
		}
	}
	return BestDirection * Manager->MaxSpeed * Manager->ObstacleAvoidanceFactor * CloseProximityMultiplier;
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
	FVector Heading = FVector::ZeroVector;
	if (waitCounter > 0) 
	{
		waitCounter -= DeltaTime;
	}

	//find velocity
	
	TArray<ABoids*> closestBoids = Manager->GetBoidNeighbourHood(this);

	Heading += Seperation(closestBoids) * Manager->SeperationWeight;
	Heading += Cohesion(closestBoids) * Manager->CohesionWeight;
	Heading += Alignment(closestBoids) * Manager->AlignmentWeight;
	
	// obstacle avoidance heading
	Heading += CalculateObstacleAvoidance();

	Velocity += Heading;

	// clamp speed and velocity
	float Speed = Velocity.Length();

	if (Speed == 0.0)
	{
		Velocity = GetActorForwardVector();
		Speed = 1.0;
	}

	if (Speed < Manager->MinSpeed)
	{
		Velocity *= Manager->MinSpeed / Speed;
	}
	else if (Speed > Manager->MaxSpeed)
	{
		Velocity *= Manager->MaxSpeed / Speed;
	}

	Heading.Normalize();

	if (Heading.Size() < 1.0f)
	{
		Heading += Wander(100.0f, 200.0f, 50.0f);
		Heading.Normalize();
	}

	FVector newForce = Heading - currentVelocity;
	currentVelocity += newForce * DeltaTime;

	FVector location = GetActorLocation();
	location += (currentVelocity * Speed * DeltaTime);

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

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Boids.generated.h"

UCLASS()
class STEERING_BEHAVIOURS_API ABoids : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABoids();

	FVector currentVelocity = FVector::ZeroVector;

	class AboidManager* Manager;

	float waitCounter = 0.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	FVector Seek(FVector position);
	FVector Flee(FVector position);

	FVector Alignment(TArray<ABoids*> Neighbours);
	FVector Cohesion(TArray<ABoids*> Neighbours);
	FVector Seperation(TArray<ABoids*> Neighbours);

	UStaticMeshComponent* Mesh;
	float Speed = 100.0f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void UpdateBoid(float DeltaTime);
};

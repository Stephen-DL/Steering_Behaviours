// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoidManager.generated.h"

UCLASS()
class STEERING_BEHAVIOURS_API ABoidManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABoidManager();

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	int SpawnCount = 30;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	float SpawnRadius = 500.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	float NeighbourRadius = 900.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	float SeperationWeight = 1.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	float CohesionWeight = 1.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Settings")
	float AlignmentWeight = 1.0f;

	USceneComponent* transform;

	TArray<class ABoids*> MyBoids;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	TArray<class ABoids*>GetBoidNeighbourHood(class ABoids* thisBoid);

};

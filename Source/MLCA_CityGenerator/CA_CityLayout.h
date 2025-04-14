// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "CA_CityLayout.generated.h"

UCLASS()
class MLCA_CITYGENERATOR_API ACA_CityLayout : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACA_CityLayout();

    static const int32 EMPTY = -1;
    static const int32 ROAD = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GridSize = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NumDistricts = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GrowthProb = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinSeedDistance = 3;

    UPROPERTY(BlueprintReadOnly)
    int32 Iterations = 0;

    UPROPERTY(BlueprintReadOnly)
    TArray<int32> Grid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    UInstancedStaticMeshComponent* InstancedGridMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 InSeed = 42;

    UFUNCTION(BlueprintCallable, CallInEditor)
    void Initialize();

    UFUNCTION(BlueprintCallable, CallInEditor)
    void Simulate();
    
    UFUNCTION(BlueprintCallable, CallInEditor)
    void VisualizeGrid();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
    FRandomStream RNG;
    TArray<FIntPoint> SeedPositions;

    int32 GetIndex(int32 X, int32 Y) const;

    bool IsInBounds(int32 X, int32 Y) const;

    int32 ManhattanDistance(FIntPoint A, FIntPoint B) const;

    void PlaceSeeds();

    TArray<FIntPoint> GetMooreNeighbors(int32 X, int32 Y) const;

    void GrowDistricts(TArray<int32>& OutGrid);

    void AddRoads(TArray<int32>& GridRef);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
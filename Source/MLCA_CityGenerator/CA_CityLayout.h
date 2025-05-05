// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "RoadStruct.h"
#include "LayerEnum.h"
#include "BuildingTypeEnum.h"
#include "DistrictStruct.h"
#include "DistrictTypeEnum.h"
#include "CA_CityLayout.generated.h"


UCLASS()
class MLCA_CITYGENERATOR_API ACA_CityLayout : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACA_CityLayout();

    static const int32 EMPTY = 0;
    static const int32 ROAD = -1;
    static const int32 JUNCTION = -2;
	static const int32 RESIDENTIAL = 1;
	static const int32 COMMERCIAL = 2;
	static const int32 INDUSTRIAL = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
    int32 GridSize = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
    int32 NumDistricts = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
    float GrowthProb = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
    int32 MinSeedDistance = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	int32 InSeed = 42;

	// Cell size for visualization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	float CellSize = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    UInstancedStaticMeshComponent* InstancedGridMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	int32 WaterSeedAmount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	int32 WaterMinSeedDistance = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	int32 WaterSpreadDistance = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Electricity")
	int32 ElectricitySpreadDistance = 7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Satisfaction")
	int32 SchoolSpreadDistance = 15;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Satisfaction")
	int32 ParkSpreadDistance = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Satisfaction")
	int32 PoliceStationSpreadDistance = 15;

	UPROPERTY(BlueprintReadOnly)
	int32 Iterations = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> Grid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FRoadStruct> RoadArray;

	UPROPERTY(EditAnywhere)
	TMap<int32, FLinearColor> DistrictColors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsMeetAJunction = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> RoadLayerGrid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> DistrictLayerGrid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> WaterLayerGrid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> ElectricityLayerGrid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> PopulationSatisfactionLayerGrid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> PolutionLayerGrid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> PopulationDensityLayerGrid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> RoadAccessibilityLayerGrid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> SecurityLayerGrid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDistrictStruct> DistrictArray;

    UFUNCTION(BlueprintCallable, CallInEditor)
    void Initialize();

    UFUNCTION(BlueprintCallable, CallInEditor)
    void Simulate();
    
    UFUNCTION(BlueprintCallable, CallInEditor)
    void VisualizeGrid();

    UFUNCTION(BlueprintCallable)
    void TrimRoads();

	UFUNCTION(BlueprintCallable)
	void RandomWalk(int32 X, int32 Y, TArray<bool>& Visited, int32 PrevIndex);

	UFUNCTION(BlueprintCallable)
    void GetRoadJunctions();

	UFUNCTION(BlueprintCallable)
    TArray<FIntPoint> GetMooreNeighbors(int32 X, int32 Y) const;

    UFUNCTION(BlueprintCallable)
    TArray<FIntPoint> GetVonNeumannNeighbors(int32 X, int32 Y) const;

	UFUNCTION(BlueprintCallable)
    void GetAllRoads();

	UFUNCTION(BlueprintCallable)
    int32 RecursiveWalkToRoads(TArray<int32>& GridArray, int32 Index, TArray<int32>& RoadIndexArray, TArray<int32>& NeighborIndex, TArray<int32>& JunctionIndex);

	UFUNCTION(BlueprintCallable)
	void AddRoadsToArray(TArray<int32>& RoadIndexArray, TArray<int32>& JunctionIndex);

	UFUNCTION(BlueprintCallable)
	void SelectionSortTheRoad();

	UFUNCTION(BlueprintCallable)
	void InitializeLayerValues();

	UFUNCTION(BlueprintCallable)
	void UpdateISMToSpecificLayer(ELayerEnum LayerEnum);

	UFUNCTION(BlueprintCallable)
	void AddBuildingEffects(EBuildingTypeEnum BuildingType, int32 TileLocationX, int32 TileLocationY);

	UFUNCTION(BlueprintCallable)
	void CalculateDistrictType();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
    FRandomStream RNG;
    TArray<FIntPoint> SeedPositions;

    int32 GetIndex(int32 X, int32 Y) const;

    bool IsInBounds(int32 X, int32 Y) const;

    int32 ManhattanDistance(FIntPoint A, FIntPoint B) const;

    void PlaceSeeds();

    void GrowDistricts(TArray<int32>& OutGrid);

    void AddRoads(TArray<int32>& GridRef);

	void PatchEmptyCells();

	TArray<FIntPoint> GetVonNeumannNeighborsWithinRadius(int32 StartX, int32 StartY, int32 Radius) const;

	TArray<FIntPoint> GetMooreNeighborsWithinRadius(int32 StartX, int32 StartY, int32 Radius) const;

	void InitializeRoadLayerGridValues();

	void InitializeDistrictLayerGridValues();

	void InitializeWaterLayerGridValues();

	void InitializeElectricityLayerGridValues();

	void InitializePopulationSatisfactionLayerGridValues();

	void InitializePolutionLayerGridValues();

	void InitializePopulationDensityLayerGridValues();

	void InitializeRoadAccessibilityLayerGridValues();

	void InitializeSecurityLayerGridValues();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
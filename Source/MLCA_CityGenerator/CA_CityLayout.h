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
#include "BlockCellStruct.h"
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
	static const int32 YARD = 4;

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

	// Residential building size
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	int32 minXSizeResidental = 5;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	int32 minYSizeResidental = 5;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	int32 maxXSizeResidental = 7;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	int32 maxYSizeResidental = 7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	int32 minHeightResidential = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	int32 maxHeightResidential = 3;

	// Commercial building size
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	int32 minXSizeCommercial = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	int32 minYSizeCommercial = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	int32 maxXSizeCommercial = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	int32 maxYSizeCommercial = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	int32 minHeightCommercial = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	int32 maxHeightCommercial = 4;


	// Industrial building size
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	int32 minXSizeIndustrial = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	int32 minYSizeIndustrial = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	int32 maxXSizeIndustrial = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	int32 maxYSizeIndustrial = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	int32 minHeightIndustrial = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	int32 maxHeightIndustrial = 6;

	// Instanced Static Mesh Component for visualization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    UInstancedStaticMeshComponent* InstancedGridMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	UInstancedStaticMeshComponent* ISMBlocks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	UInstancedStaticMeshComponent* ISMTree;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	UInstancedStaticMeshComponent* ISMTreeV2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	UInstancedStaticMeshComponent* ISMTreeV3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	UInstancedStaticMeshComponent* ISMTile;

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

	// Thresholds for each layer
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threshold")
	int32 WaterThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threshold")
	int32 ElectricityThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threshold")
	int32 PopulationSatisfactionThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threshold")
	int32 PolutionThreshold = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threshold")
	int32 PopulationDensityThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threshold")
	int32 RoadAccessibilityThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threshold")
	int32 SecurityThreshold = 0;


	// Utility
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

	// Layers Array
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
	TArray<int32> TreeLayerArray;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> TileLayerArray;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> BuildingLayerArray;

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

	UFUNCTION(BlueprintCallable)
	void GetAllDistricts();

	UFUNCTION(BlueprintCallable, Category = "Partition")
	void PartitionGridBSP(TArray<int32> DistrictIndex, int DistrictType);

	UFUNCTION(BlueprintCallable)
	void FloorPlanAllDistricts();

	UFUNCTION(BlueprintCallable)
	void VisualizeTheBlocks();

	UFUNCTION(BlueprintCallable)
	void GetAllLargestRectanglesForDistricts();

	UFUNCTION(BlueprintCallable)
	void SetBuildingLayerGridValues();

	UFUNCTION(BlueprintCallable)
	void SetTreeLayerGridValues();

	UFUNCTION(BlueprintCallable)
	void SetTileLayerGridValues();



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
    FRandomStream RNG;
    TArray<FIntPoint> SeedPositions;
	int32 minBlockValue;
	int32 maxBlockValue;
	int32 CurrentDistrict = 0;
	int32 CellCount = 0;

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

	void DoBSP_Grid(int32 X, int32 Y, int32 Width, int32 Height, int32& NextID, TArray<int32>& BlockIndex, int DistrictType);

	int32 GetMaxHeight(TArray<int32>& GridArray, int32 Y);

	int32 GetMaxWidth(TArray<int32>& GridArray, int32 X);

	int32 GetMinX(TArray<int32>& GridArray);

	int32 GetMinY(TArray<int32>& GridArray);

	void FindLargestRectangle(TArray<int32>& ComponentIndices,int32& OutSizeX, int32& OutSizeY, int DistrictType);


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
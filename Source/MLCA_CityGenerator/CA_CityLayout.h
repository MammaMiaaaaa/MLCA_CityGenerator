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
	static const int32 URBAN_FOREST = 4;

	// Building Direction
	static const int32 LEFT = 1;
	static const int32 TOP = 2;
	static const int32 RIGHT = 3;
	static const int32 BOTTOM = 4;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	int32 AdditionalRoadWidth = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	bool bUseMLCAValues = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	bool bOrganizedSeedPlacement = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	bool bSynchronousUpdate = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	bool bUseMooreNeighborhood = true;

	//District Weights
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DistrictWeight")
	int32 ResidentialDistrictWeight = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DistrictWeight")
	int32 CommercialDistrictWeight = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DistrictWeight")
	int32 IndustrialDistrictWeight = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DistrictWeight")
	int32 UrbanForestDistrictWeight = 1;



	// Residential building size
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	int32 minXSizeResidental = 5;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	int32 minYSizeResidental = 5;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	int32 maxXSizeResidental = 7;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	int32 maxYSizeResidental = 7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	int32 minHeightResidential = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	int32 maxHeightResidential = 3;

	// Commercial building size
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	int32 minXSizeCommercial = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	int32 minYSizeCommercial = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	int32 maxXSizeCommercial = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	int32 maxYSizeCommercial = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	int32 minHeightCommercial = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	int32 maxHeightCommercial = 15;


	// Industrial building size
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	int32 minXSizeIndustrial = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	int32 minYSizeIndustrial = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	int32 maxXSizeIndustrial = 12;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	int32 maxYSizeIndustrial = 12;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	int32 minHeightIndustrial = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	int32 maxHeightIndustrial = 5;

	// Instanced Static Mesh Component for visualization
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Visual")
    UInstancedStaticMeshComponent* InstancedGridMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Visual")
	UInstancedStaticMeshComponent* ISMBlocks;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Visual")
	UInstancedStaticMeshComponent* ISMTree;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Visual")
	UInstancedStaticMeshComponent* ISMTreeV2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Visual")
	UInstancedStaticMeshComponent* ISMTreeV3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Visual")
	UInstancedStaticMeshComponent* ISMTile;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Visual")
	UInstancedStaticMeshComponent* ISMGrass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Visual")
	UInstancedStaticMeshComponent* ISMGrassV2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Visual")
	UInstancedStaticMeshComponent* ISMBush;

	// Cell size for visualization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	float CellSize = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	int32 WaterSeedAmount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	int32 WaterMinSeedDistance = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	int32 WaterSpreadDistance = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Electricity")
	int32 ElectricitySpreadDistance = 15;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Satisfaction")
	int32 SchoolSpreadDistance = 15;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Satisfaction")
	int32 ParkSpreadDistance = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Satisfaction")
	int32 PoliceStationSpreadDistance = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoadAccess")
	int32 MinRoadAccessDistance = 3;

	// Residential Thresholds for each layer
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ResidentialThreshold")
	int32 ResidentialWaterThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ResidentialThreshold")
	int32 ResidentialElectricityThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ResidentialThreshold")
	int32 ResidentialPopulationSatisfactionThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ResidentialThreshold")
	int32 ResidentialPolutionThreshold = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ResidentialThreshold")
	int32 ResidentialPopulationDensityThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ResidentialThreshold")
	int32 ResidentialRoadAccessibilityThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ResidentialThreshold")
	int32 ResidentialSecurityThreshold = 0;

	// Commercial Thresholds for each layer
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CommercialThreshold")
	int32 CommercialWaterThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CommercialThreshold")
	int32 CommercialElectricityThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CommercialThreshold")
	int32 CommercialPopulationSatisfactionThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CommercialThreshold")
	int32 CommercialPolutionThreshold = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CommercialThreshold")
	int32 CommercialPopulationDensityThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CommercialThreshold")
	int32 CommercialRoadAccessibilityThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CommercialThreshold")
	int32 CommercialSecurityThreshold = 0;

	// Industrial Thresholds for each layer
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IndustrialThreshold")
	int32 IndustrialWaterThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IndustrialThreshold")
	int32 IndustrialElectricityThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IndustrialThreshold")
	int32 IndustrialPopulationSatisfactionThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IndustrialThreshold")
	int32 IndustrialPolutionThreshold = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IndustrialThreshold")
	int32 IndustrialPopulationDensityThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IndustrialThreshold")
	int32 IndustrialRoadAccessibilityThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IndustrialThreshold")
	int32 IndustrialSecurityThreshold = 0;

	// UrbanForest Thresholds for each layer
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UrbanForestThreshold")
	int32 UrbanForestWaterThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UrbanForestThreshold")
	int32 UrbanForestElectricityThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UrbanForestThreshold")
	int32 UrbanForestPopulationSatisfactionThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UrbanForestThreshold")
	int32 UrbanForestPolutionThreshold = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UrbanForestThreshold")
	int32 UrbanForestPopulationDensityThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UrbanForestThreshold")
	int32 UrbanForestRoadAccessibilityThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UrbanForestThreshold")
	int32 UrbanForestSecurityThreshold = 0;


	
	// Utility
	UPROPERTY(BlueprintReadOnly)
	int32 Iterations = 0;

	UPROPERTY(BlueprintReadWrite)
	TArray<FRoadStruct> RoadArray;

	TMap<int32, FLinearColor> DistrictColors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsMeetAJunction = false;

	UPROPERTY(BlueprintReadWrite)
	TArray<int32> Grid;

	// Layers Array
	UPROPERTY(BlueprintReadWrite)
    TArray<int32> RoadLayerGrid;

	UPROPERTY(BlueprintReadWrite)
	TArray<int32> DistrictLayerGrid;

	UPROPERTY(BlueprintReadWrite)
    TArray<int32> WaterLayerGrid;

	UPROPERTY(BlueprintReadWrite)
	TArray<int32> ElectricityLayerGrid;

	UPROPERTY(BlueprintReadWrite)
	TArray<int32> PopulationSatisfactionLayerGrid;

	UPROPERTY(BlueprintReadWrite)
	TArray<int32> PolutionLayerGrid;

	UPROPERTY(BlueprintReadWrite)
	TArray<int32> PopulationDensityLayerGrid;

	UPROPERTY(BlueprintReadWrite)
	TArray<int32> RoadAccessibilityLayerGrid;

	UPROPERTY(BlueprintReadWrite)
	TArray<int32> SecurityLayerGrid;

	UPROPERTY(BlueprintReadWrite)
	TArray<int32> TreeLayerArray;

	UPROPERTY(BlueprintReadWrite)
	TArray<int32> TileLayerArray;

	UPROPERTY(BlueprintReadWrite)
	TArray<int32> BuildingLayerArray;

	UPROPERTY(BlueprintReadWrite)
	TArray<FDistrictStruct> DistrictArray;

    UFUNCTION(BlueprintCallable, CallInEditor)
    void Initialize();

    UFUNCTION(BlueprintCallable, CallInEditor)
    void Simulate();
    
    UFUNCTION(BlueprintCallable, CallInEditor)
    void VisualizeGrid();

	UFUNCTION(BlueprintCallable)
	void AddRoads(TArray<int32>& GridRef);

	UFUNCTION(BlueprintCallable)
	void PatchEmptyCells();

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
	void CalculateTileLayerGridValues();

	UFUNCTION(BlueprintCallable)
	void AddRoadWidth();

	UFUNCTION(BlueprintCallable)
	void CalculateRoadType();

	UFUNCTION(BlueprintCallable)
	void SetBuildingDirection();



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

	TArray<FIntPoint> GetVonNeumannNeighborsWithinRadius(int32 StartX, int32 StartY, int32 Radius) const;

	TArray<FIntPoint> GetMooreNeighborsWithinRadius(int32 StartX, int32 StartY, int32 Radius) const;

	bool HasNeighboringRoad(int32 StartX, int32 StartY, int32 Radius) const;

	void InitializeRoadLayerGridValues();

	void InitializeDistrictLayerGridValues();

	void InitializeWaterLayerGridValues();

	void InitializeElectricityLayerGridValues();

	void InitializePopulationSatisfactionLayerGridValues();

	void InitializePolutionLayerGridValues();

	void InitializePopulationDensityLayerGridValues();

	void InitializeRoadAccessibilityLayerGridValues();

	void InitializeSecurityLayerGridValues();
		
	void InitializeBuildingLayerGridValues();

	void DoBSP_Grid(int32 X, int32 Y, int32 Width, int32 Height, int32& NextID, TArray<int32>& BlockIndex, int DistrictType);

	int32 GetMaxHeight(TArray<int32>& GridArray, int32 Y);

	int32 GetMaxWidth(TArray<int32>& GridArray, int32 X);

	int32 GetMinX(TArray<int32>& GridArray);

	int32 GetMinY(TArray<int32>& GridArray);

	void FindLargestRectangle(TArray<int32>& ComponentIndices,int32& OutSizeX, int32& OutSizeY, int DistrictType);

	int32 GetRoadType(int32 RoadIndex) const;

	int32 GetMinDistanceToRoad(int32 X, int32 Y) const;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
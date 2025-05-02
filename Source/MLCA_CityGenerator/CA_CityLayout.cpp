// Fill out your copyright notice in the Description page of Project Settings.


#include "CA_CityLayout.h"
#include "Containers/UnrealString.h"
#include "Engine/World.h"

// Sets default values
ACA_CityLayout::ACA_CityLayout()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    // Create the instanced static mesh component and attach it
    InstancedGridMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedGridMesh"));
    RootComponent = InstancedGridMesh;

    // We want 3 floats per instance (R, G, B, A).
    InstancedGridMesh->NumCustomDataFloats = 4;

}

// Called when the game starts or when spawned
void ACA_CityLayout::BeginPlay()
{
	Super::BeginPlay();

	
}

// Called every frame
void ACA_CityLayout::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
int32 ACA_CityLayout::GetIndex(int32 X, int32 Y) const
{
    return Y * GridSize + X;
}

bool ACA_CityLayout::IsInBounds(int32 X, int32 Y) const
{
    return X >= 0 && Y >= 0 && X < GridSize && Y < GridSize;
}

int32 ACA_CityLayout::ManhattanDistance(FIntPoint A, FIntPoint B) const
{
    return FMath::Abs(A.X - B.X) + FMath::Abs(A.Y - B.Y);
}

void ACA_CityLayout::Initialize()
{
    Grid.Init(EMPTY, GridSize * GridSize);
    RNG.Initialize(InSeed);
    PlaceSeeds();
}

void ACA_CityLayout::PlaceSeeds()
{
    SeedPositions.Empty();
    int32 Attempts = 0;
    const int32 MaxAttempts = GridSize * GridSize;

    while (SeedPositions.Num() < NumDistricts && Attempts < MaxAttempts)
    {
        int32 X = RNG.RandRange(0, GridSize - 1);
        int32 Y = RNG.RandRange(0, GridSize - 1);
        FIntPoint Pos(X, Y);

        bool IsValid = true;
        for (const auto& Seed : SeedPositions)
        {
            if (ManhattanDistance(Seed, Pos) < MinSeedDistance)
            {
                IsValid = false;
                break;
            }
        }

        if (IsValid)
        {
            SeedPositions.Add(Pos);
            Grid[GetIndex(X, Y)] = SeedPositions.Num(); // District IDs start from 1
        }

        Attempts++;
    }

    if (SeedPositions.Num() < NumDistricts)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to place all district seeds."));
    }
}

TArray<FIntPoint> ACA_CityLayout::GetMooreNeighbors(int32 X, int32 Y) const
{
    TArray<FIntPoint> Neighbors;
    for (int32 dx = -1; dx <= 1; ++dx)
    {
        for (int32 dy = -1; dy <= 1; ++dy)
        {
            if (dx == 0 && dy == 0) continue;
            int32 nx = X + dx;
            int32 ny = Y + dy;
            if (IsInBounds(nx, ny))
            {
                Neighbors.Add(FIntPoint(nx, ny));
            }
        }
    }
    return Neighbors;
}

TArray<FIntPoint> ACA_CityLayout::GetVonNeumannNeighbors(int32 X, int32 Y) const
{
	TArray<FIntPoint> Neighbors;
	// Von Neumann neighbors are the four orthogonal directions
	const int32 dx[4] = { -1, 0, 1, 0 };
	const int32 dy[4] = { 0, -1, 0, 1 };
	for (int32 i = 0; i < 4; ++i)
	{
		int32 nx = X + dx[i];
		int32 ny = Y + dy[i];
		if (IsInBounds(nx, ny))
		{
			Neighbors.Add(FIntPoint(nx, ny));
		}
	}



    return Neighbors;
}

void ACA_CityLayout::GetAllRoads()
{
	TArray<int32> TempGrid = Grid;
	TArray<int32> RoadIndexArray;
    TArray<int32> NeighborIndex;
	TArray<int32> JunctionIndex;
	// For each cell in the grid, check if it's a road
	for (int32 y = 0; y < GridSize; ++y)
	{
		for (int32 x = 0; x < GridSize; ++x)
		{
			int32 Index = GetIndex(x, y);
			if (TempGrid[Index] == ROAD)
			{
				NeighborIndex.Add(Index);
				// While the NeighborIndex is not empty, loop through it and check if it's a road or junction
                int32 Counter = 0;
				int32 NeighborCount = 0;
				while (NeighborIndex.Num() > 0) {
					int32 CurrentIndex = NeighborIndex[Counter];
					// Check if the current index is a junction
                    if (TempGrid[CurrentIndex] == JUNCTION) {
						NeighborIndex.Remove(CurrentIndex);
                    }
                    else {
						NeighborCount=RecursiveWalkToRoads(TempGrid, CurrentIndex, RoadIndexArray, NeighborIndex,JunctionIndex);
                    }
					Counter++;
					
					if (NeighborCount != 0) {
						// Reset the counter to 0
						Counter = 0;
					}
					if (Counter >= NeighborIndex.Num()) {
						Counter = 0;
					}
				}
				AddRoadsToArray(RoadIndexArray, JunctionIndex);
			}
		}
	}
}

int32 ACA_CityLayout::RecursiveWalkToRoads(TArray<int32>& GridArray, int32 Index, TArray<int32>& RoadIndexArray, TArray<int32>& NeighborIndexArray, TArray<int32>& JunctionIndex)
{
    TArray<int32> TempNeighbor;
    IsMeetAJunction = false;
	// Set the current cell as visited
	GridArray[Index] = -3; // Mark as visited
	RoadIndexArray.Add(Index);
    NeighborIndexArray.Remove(Index);
	// Clear TempNeighbor
	TempNeighbor.Empty();

	// For each neighbor, check if it's a road
	TArray<FIntPoint> Neighbors = GetMooreNeighbors(Index % GridSize, Index / GridSize);
	for (const auto& N : Neighbors)
	{
		int32 NeighborIndex = GetIndex(N.X, N.Y);
		if (GridArray[NeighborIndex] == ROAD)
		{
			TempNeighbor.Add(NeighborIndex);
		}
		else if (GridArray[NeighborIndex] == JUNCTION)
		{
			IsMeetAJunction = true;
			NeighborIndexArray.Add(NeighborIndex);
			RoadIndexArray.AddUnique(NeighborIndex);
			JunctionIndex.AddUnique(NeighborIndex);
		}
	}
    // If meet a junction, remove the diagonal neighbors road cell
    if (IsMeetAJunction)
    {
		// For each temp neighbor, check if it's a von Neurmann neighbor
        for (const auto& N : TempNeighbor) {
			if (N == Index - 1 || N == Index + 1 || N == Index - GridSize || N == Index + GridSize)
			{
				// If it's a diagonal neighbor, remove it from the NeighborIndexArray
                NeighborIndexArray.AddUnique(N);
			}
        }
        
    }
    else {
        for (const auto& N : TempNeighbor) {
			NeighborIndexArray.AddUnique(N);
        }
    }
	return NeighborIndexArray.Num();
}

void ACA_CityLayout::AddRoadsToArray(TArray<int32>& RoadIndexArray, TArray<int32>& JunctionIndex)
{
    if (!InstancedGridMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("InstancedGridMesh is null in AddRoadsToArray."));
        return;
    }

    // Create a new vector array to store the road indices
    TArray<FVector> RoadPointsLocation;
    RoadPointsLocation.Reserve(RoadIndexArray.Num());

    // Get InstancedGridMesh location and store to RoadPointsLocation
    for (int32 Index : RoadIndexArray)
    {
        if (InstancedGridMesh->GetInstanceCount() > Index)
        {
            FTransform InstanceTransform;
            InstancedGridMesh->GetInstanceTransform(Index, InstanceTransform, true);
            RoadPointsLocation.Add(InstanceTransform.GetLocation());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid index %d in RoadIndexArray"), Index);
        }
    }
	// Add the road points to the RoadArray
	FRoadStruct RoadStruct;
	RoadStruct.RoadPointsLocation = RoadPointsLocation;
	RoadStruct.RoadPointsIndex = RoadIndexArray;
	RoadStruct.JunctionIndex = JunctionIndex;
	RoadArray.Add(RoadStruct);
	// Clear the RoadIndexArray for the next road
	RoadIndexArray.Empty();
	// Clear the JunctionIndex for the next road
	JunctionIndex.Empty();


}

void ACA_CityLayout::SelectionSortTheRoad()
{
    for (FRoadStruct& Road : RoadArray)
    {
		TArray<FVector>& Points = Road.RoadPointsLocation;
		TArray<int32>& Indices = Road.RoadPointsIndex;
        int32 Num = Points.Num();
        
		// Check if the road has junctions
		if (Road.JunctionIndex.Num() > 0)
		{
            int32 Index = Road.JunctionIndex[0];
			// Find the first index of the road points that is equal to the junction index
			int32 FirstIndex = 0;
			for (int32 i = 0; i < Num; ++i)
			{
				if (Indices[i] == Index)
				{
					FirstIndex = i;
					break;
				}
			}
			Points.Swap(0, FirstIndex);
			Indices.Swap(0, FirstIndex);
		}
		
        for (int32 i = 1; i < Num - 1; ++i)
        {
			float Distance = FVector::Dist(Points[i-1], Points[i]);
            int32 MinIndex = i;
			
            for (int32 j = i + 1; j < Num; ++j)
            {
				float Dist = FVector::Dist(Points[i-1], Points[j]);
				if (Dist < Distance)
				{
					Distance = Dist;
					MinIndex = j;
				}
                
            }
            if (MinIndex != i)
            {
                Points.Swap(i, MinIndex);
				Indices.Swap(i, MinIndex);
            }
        }
		Road.RoadPointsLocation = Points;
		Road.RoadPointsIndex = Indices;
    }


    
}

void ACA_CityLayout::InitializeLayerValues()
{
    InitializeRoadLayerGridValues();
    InitializeDistrictLayerGridValues();
    InitializeWaterLayerGridValues();
    InitializeElectricityLayerGridValues();
    InitializePopulationSatisfactionLayerGridValues();
    InitializePolutionLayerGridValues();
    InitializePopulationDensityLayerGridValues();
    InitializeRoadAccessibilityLayerGridValues();
    InitializeSecurityLayerGridValues();
}

void ACA_CityLayout::UpdateLayerValues(ELayerEnum LayerEnum)
{
    switch (LayerEnum)
    {
    case ELayerEnum::Road:

        break;
    case ELayerEnum::District:
        break;
    case ELayerEnum::Water:
        break;
    case ELayerEnum::Electricity:
        break;
    case ELayerEnum::Satisfaction:
        break;
    case ELayerEnum::Polution:
        break;
    case ELayerEnum::Density:
        break;
    case ELayerEnum::Accessibility:

        break;
    case ELayerEnum::Security:

        break;
    default:
        UE_LOG(LogTemp, Warning, TEXT("UpdateLayerValues: Unknown LayerEnum value!"));
        break;
    }
}

void ACA_CityLayout::UpdateISMToSpecificLayer(ELayerEnum LayerEnum)
{
    FLinearColor GrayColor = FLinearColor::Gray;
    FLinearColor BlackColor = FLinearColor::Black;
    FLinearColor GreenColor = FLinearColor::Green;
    FLinearColor BlueColor = FLinearColor::Blue;
    FLinearColor YellowColor = FLinearColor::Yellow;
    switch (LayerEnum)
    {
    case ELayerEnum::Road:
		// for loop through the RoadLayerGrid and set the material color according to the value
        
		for (int32 y = 0; y < GridSize; ++y)
		{
			for (int32 x = 0; x < GridSize; ++x)
			{
				int32 Index = GetIndex(x, y);
				if (RoadLayerGrid[Index] == ROAD)
				{
                    InstancedGridMesh->SetCustomDataValue(Index, 0, BlackColor.R);
                    InstancedGridMesh->SetCustomDataValue(Index, 1, BlackColor.G);
                    InstancedGridMesh->SetCustomDataValue(Index, 2, BlackColor.B);
                    InstancedGridMesh->SetCustomDataValue(Index, 3, 1);
                    InstancedGridMesh->SetCustomDataValue(Index, 4, 0.5);
				}
                else {
					InstancedGridMesh->SetCustomDataValue(Index, 0, GrayColor.R);
					InstancedGridMesh->SetCustomDataValue(Index, 1, GrayColor.G);
					InstancedGridMesh->SetCustomDataValue(Index, 2, GrayColor.B);
                    InstancedGridMesh->SetCustomDataValue(Index, 3, 1);
                    InstancedGridMesh->SetCustomDataValue(Index, 4, 0);
                }
			}
		}
        break;
    case ELayerEnum::District:
        // for loop through the DistrictLayerGrid and set the material color according to the value
        for (int32 y = 0; y < GridSize; ++y)
        {
            for (int32 x = 0; x < GridSize; ++x)
            {
                int32 Index = GetIndex(x, y);
                if (DistrictLayerGrid[Index] == RESIDENTIAL)
                {
                    InstancedGridMesh->SetCustomDataValue(Index, 0, GreenColor.R);
                    InstancedGridMesh->SetCustomDataValue(Index, 1, GreenColor.G);
                    InstancedGridMesh->SetCustomDataValue(Index, 2, GreenColor.B);
                    InstancedGridMesh->SetCustomDataValue(Index, 3, 1);
                    InstancedGridMesh->SetCustomDataValue(Index, 4, 0.5);
                }
                else if(DistrictLayerGrid[Index] == COMMERCIAL) {
                    InstancedGridMesh->SetCustomDataValue(Index, 0, BlueColor.R);
                    InstancedGridMesh->SetCustomDataValue(Index, 1, BlueColor.G);
                    InstancedGridMesh->SetCustomDataValue(Index, 2, BlueColor.B);
                    InstancedGridMesh->SetCustomDataValue(Index, 3, 1);
                    InstancedGridMesh->SetCustomDataValue(Index, 4, 0.5);
                }
				else if (DistrictLayerGrid[Index] == INDUSTRIAL) {
					InstancedGridMesh->SetCustomDataValue(Index, 0, YellowColor.R);
					InstancedGridMesh->SetCustomDataValue(Index, 1, YellowColor.G);
					InstancedGridMesh->SetCustomDataValue(Index, 2, YellowColor.B);
                    InstancedGridMesh->SetCustomDataValue(Index, 3, 1);
                    InstancedGridMesh->SetCustomDataValue(Index, 4, 0.5);
				}
				else if (DistrictLayerGrid[Index] == ROAD) {
					InstancedGridMesh->SetCustomDataValue(Index, 0, BlackColor.R);
					InstancedGridMesh->SetCustomDataValue(Index, 1, BlackColor.G);
					InstancedGridMesh->SetCustomDataValue(Index, 2, BlackColor.B);
                    InstancedGridMesh->SetCustomDataValue(Index, 3, 1);
                    InstancedGridMesh->SetCustomDataValue(Index, 4, 0.5);
				}
				else if (DistrictLayerGrid[Index] == EMPTY) {
					InstancedGridMesh->SetCustomDataValue(Index, 0, GrayColor.R);
					InstancedGridMesh->SetCustomDataValue(Index, 1, GrayColor.G);
					InstancedGridMesh->SetCustomDataValue(Index, 2, GrayColor.B);
                    InstancedGridMesh->SetCustomDataValue(Index, 3, 1);
                    InstancedGridMesh->SetCustomDataValue(Index, 4, 0);
				}
				else {
					InstancedGridMesh->SetCustomDataValue(Index, 0, GrayColor.R);
					InstancedGridMesh->SetCustomDataValue(Index, 1, GrayColor.G);
					InstancedGridMesh->SetCustomDataValue(Index, 2, GrayColor.B);
                    InstancedGridMesh->SetCustomDataValue(Index, 3, 1);
                    InstancedGridMesh->SetCustomDataValue(Index, 4, 0);
				}
            }
        }
        break;
    case ELayerEnum::Water:
        // for loop through the WaterLayerGrid and set the material color according to the value
        for (int32 y = 0; y < GridSize; ++y)
        {
            for (int32 x = 0; x < GridSize; ++x)
            {
                int32 Index = GetIndex(x, y);
                if (WaterLayerGrid[Index] > 0)
                {
                    InstancedGridMesh->SetCustomDataValue(Index, 0, BlueColor.R);
                    InstancedGridMesh->SetCustomDataValue(Index, 1, BlueColor.G);
                    InstancedGridMesh->SetCustomDataValue(Index, 2, BlueColor.B);
                    // Set Custom Data Value 3 acording to the WaterLayerGrid Value
                    InstancedGridMesh->SetCustomDataValue(Index, 3, static_cast<float>(WaterLayerGrid[Index]) / 100.0);
                    InstancedGridMesh->SetCustomDataValue(Index, 4, 0.5);

                }
                else {
					InstancedGridMesh->SetCustomDataValue(Index, 0, GrayColor.R);
					InstancedGridMesh->SetCustomDataValue(Index, 1, GrayColor.G);
					InstancedGridMesh->SetCustomDataValue(Index, 2, GrayColor.B);
                    InstancedGridMesh->SetCustomDataValue(Index, 3, 1);
                    InstancedGridMesh->SetCustomDataValue(Index, 4, 0);
                }
            }
        }
        break;
    case ELayerEnum::Electricity:
		// for loop through the ElectricityLayerGrid and set the material color according to the value
        for (int32 y = 0; y < GridSize; ++y)
        {
            for (int32 x = 0; x < GridSize; ++x)
            {
                int32 Index = GetIndex(x, y);
                if (ElectricityLayerGrid[Index] > 0)
                {
                    InstancedGridMesh->SetCustomDataValue(Index, 0, YellowColor.R);
                    InstancedGridMesh->SetCustomDataValue(Index, 1, YellowColor.G);
                    InstancedGridMesh->SetCustomDataValue(Index, 2, YellowColor.B);
                    InstancedGridMesh->SetCustomDataValue(Index, 3, static_cast<float>(ElectricityLayerGrid[Index]) / 100.0);
                    InstancedGridMesh->SetCustomDataValue(Index, 4, 0.5);
                }
                else {
					InstancedGridMesh->SetCustomDataValue(Index, 0, GrayColor.R);
					InstancedGridMesh->SetCustomDataValue(Index, 1, GrayColor.G);
					InstancedGridMesh->SetCustomDataValue(Index, 2, GrayColor.B);
                    InstancedGridMesh->SetCustomDataValue(Index, 3, 1);
                    InstancedGridMesh->SetCustomDataValue(Index, 4, 0);
                }
            }
        }
        break;
    case ELayerEnum::Satisfaction:
		// for loop through the PopulationSatisfactionLayerGrid and set the material color according to the value
        for (int32 y = 0; y < GridSize; ++y)
        {
            for (int32 x = 0; x < GridSize; ++x)
            {
                int32 Index = GetIndex(x, y);
                if (PopulationSatisfactionLayerGrid[Index] > 0)
                {
                    InstancedGridMesh->SetCustomDataValue(Index, 0, GreenColor.R);
                    InstancedGridMesh->SetCustomDataValue(Index, 1, GreenColor.G);
                    InstancedGridMesh->SetCustomDataValue(Index, 2, GreenColor.B);
                    InstancedGridMesh->SetCustomDataValue(Index, 3, static_cast<float>(PopulationSatisfactionLayerGrid[Index]) / 100.0);
                    InstancedGridMesh->SetCustomDataValue(Index, 4, 0.5);
                }
                else {
                    InstancedGridMesh->SetCustomDataValue(Index, 0, GrayColor.R);
                    InstancedGridMesh->SetCustomDataValue(Index, 1, GrayColor.G);
                    InstancedGridMesh->SetCustomDataValue(Index, 2, GrayColor.B);
                    InstancedGridMesh->SetCustomDataValue(Index, 3, 1);
                    InstancedGridMesh->SetCustomDataValue(Index, 4, 0);
                }
            }
        }
        break;
    case ELayerEnum::Polution:
        // for loop through the PolutionLayerGrid and set the material color according to the value
        for (int32 y = 0; y < GridSize; ++y)
        {
            for (int32 x = 0; x < GridSize; ++x)
            {
                int32 Index = GetIndex(x, y);
                if (PolutionLayerGrid[Index] > 0)
                {
                    InstancedGridMesh->SetCustomDataValue(Index, 0, YellowColor.R);
                    InstancedGridMesh->SetCustomDataValue(Index, 1, YellowColor.G);
                    InstancedGridMesh->SetCustomDataValue(Index, 2, YellowColor.B);
                    InstancedGridMesh->SetCustomDataValue(Index, 3, static_cast<float>(PolutionLayerGrid[Index]) / 100.0);
                    InstancedGridMesh->SetCustomDataValue(Index, 4, 0.5);
                }
                else {
                    InstancedGridMesh->SetCustomDataValue(Index, 0, GrayColor.R);
                    InstancedGridMesh->SetCustomDataValue(Index, 1, GrayColor.G);
                    InstancedGridMesh->SetCustomDataValue(Index, 2, GrayColor.B);
                    InstancedGridMesh->SetCustomDataValue(Index, 3, 1);
                    InstancedGridMesh->SetCustomDataValue(Index, 4, 0);
                }
            }
        }
        break;
    case ELayerEnum::Density:

        break;
    case ELayerEnum::Accessibility:

        break;
    case ELayerEnum::Security:
        // for loop through the PolutionLayerGrid and set the material color according to the value
        for (int32 y = 0; y < GridSize; ++y)
        {
            for (int32 x = 0; x < GridSize; ++x)
            {
                int32 Index = GetIndex(x, y);
                if (SecurityLayerGrid[Index] > 0)
                {
                    InstancedGridMesh->SetCustomDataValue(Index, 0, BlueColor.R);
                    InstancedGridMesh->SetCustomDataValue(Index, 1, BlueColor.G);
                    InstancedGridMesh->SetCustomDataValue(Index, 2, BlueColor.B);
                    InstancedGridMesh->SetCustomDataValue(Index, 3, static_cast<float>(SecurityLayerGrid[Index]) / 100.0);
                    InstancedGridMesh->SetCustomDataValue(Index, 4, 0.5);
                }
                else {
                    InstancedGridMesh->SetCustomDataValue(Index, 0, GrayColor.R);
                    InstancedGridMesh->SetCustomDataValue(Index, 1, GrayColor.G);
                    InstancedGridMesh->SetCustomDataValue(Index, 2, GrayColor.B);
                    InstancedGridMesh->SetCustomDataValue(Index, 3, 1);
                    InstancedGridMesh->SetCustomDataValue(Index, 4, 0);
                }
            }
        }
        break;
    case ELayerEnum::Grid:
        
        for (int32 y = 0; y < GridSize; ++y)
        {
            for (int32 x = 0; x < GridSize; ++x)
            {
				int32 Index = GetIndex(x, y);
                const int32 CellValue = Grid[GetIndex(x, y)];
                // Skip empty cells
                if (CellValue == EMPTY)
                {
                    continue;
                }

                // If you have a color in DistrictColors, set it
                if (DistrictColors.Contains(CellValue))
                {
                    FLinearColor Color = DistrictColors[CellValue];
                    // Write R,G,B to custom data
                    InstancedGridMesh->SetCustomDataValue(Index, 0, Color.R);
                    InstancedGridMesh->SetCustomDataValue(Index, 1, Color.G);
                    InstancedGridMesh->SetCustomDataValue(Index, 2, Color.B);
                    InstancedGridMesh->SetCustomDataValue(Index, 3, 1);
                    InstancedGridMesh->SetCustomDataValue(Index, 4, 0.5);
                }
            }
        }
        break;
    default:
        UE_LOG(LogTemp, Warning, TEXT("UpdateLayerValues: Unknown LayerEnum value!"));
        break;
    }
}

void ACA_CityLayout::AddBuildingEffects(EBuildingTypeEnum BuildingType, int32 TileLocationX, int32 TileLocationY)
{
    TArray<FIntPoint> Neighbors;
	// Add building effects based on the building type
	switch (BuildingType)
	{
	case EBuildingTypeEnum::WaterTower:
		// Add water tower effects
        WaterLayerGrid[GetIndex(TileLocationX, TileLocationY)] = FMath::Clamp(WaterLayerGrid[GetIndex(TileLocationX, TileLocationY)] + 50, 0, 100);
        Neighbors = GetVonNeumannNeighborsWithinRadius(TileLocationX, TileLocationY, WaterSpreadDistance);
        // for each Neighbors add the waterlayergrid value with 50 and clamp the value to 100
        for (const auto& N : Neighbors)
        {
            // Add the WaterLayerGrid on that index with 50 and Clamp the value to 100
            WaterLayerGrid[GetIndex(N.X, N.Y)] = FMath::Clamp(WaterLayerGrid[GetIndex(N.X, N.Y)] + 50, 0, 100);
        }
		break;
	case EBuildingTypeEnum::ElectricityTower:
		// Add Electricity Tower effects
        ElectricityLayerGrid[GetIndex(TileLocationX, TileLocationY)] = FMath::Clamp(ElectricityLayerGrid[GetIndex(TileLocationX, TileLocationY)] + 50, 0, 100);
        Neighbors = GetMooreNeighborsWithinRadius(TileLocationX, TileLocationY, ElectricitySpreadDistance);
        // for each Neighbors add the ElectricityLayerGrid value with 50 and clamp the value to 100
        for (const auto& N : Neighbors)
        {
            // Add the WaterLayerGrid on that index with 50 and Clamp the value to 100
            ElectricityLayerGrid[GetIndex(N.X, N.Y)] = FMath::Clamp(ElectricityLayerGrid[GetIndex(N.X, N.Y)] + 50, 0, 100);
        }
		break;
	case EBuildingTypeEnum::School:
        // Add School  effects
        PopulationSatisfactionLayerGrid[GetIndex(TileLocationX, TileLocationY)] = FMath::Clamp(PopulationSatisfactionLayerGrid[GetIndex(TileLocationX, TileLocationY)] + 30, 0, 100);
        for (int32 X = TileLocationX; X <= TileLocationX + 5; ++X) {
			for (int32 Y = TileLocationY; Y <= TileLocationY + 3; ++Y) {
				TArray<FIntPoint> TempNeighbors = GetMooreNeighborsWithinRadius(X, Y, SchoolSpreadDistance);
				// for each TempNeighbors add AddUnique to Neighbors
				for (const auto& N : TempNeighbors)
				{
					if (IsInBounds(N.X, N.Y) && !(N.X == TileLocationX && N.Y == TileLocationY))
					{
						Neighbors.AddUnique(N);
					}
				}
			}
        }
        // for each Neighbors add the PopulationSatisfactionLayerGrid value with 50 and clamp the value to 100
        for (const auto& N : Neighbors)
        {
            // Add the PopulationSatisfactionLayerGrid on that index with 50 and Clamp the value to 100
            PopulationSatisfactionLayerGrid[GetIndex(N.X, N.Y)] = FMath::Clamp(PopulationSatisfactionLayerGrid[GetIndex(N.X, N.Y)] + 30, 0, 100);
        }
        break;
    case EBuildingTypeEnum::Park:
        // Add Park effects
        PolutionLayerGrid[GetIndex(TileLocationX, TileLocationY)] = FMath::Clamp(PolutionLayerGrid[GetIndex(TileLocationX, TileLocationY)] - 50, 0, 100);
        for (int32 X = TileLocationX; X <= TileLocationX + 3; ++X) {
            for (int32 Y = TileLocationY; Y <= TileLocationY + 2; ++Y) {
                TArray<FIntPoint> TempNeighbors = GetMooreNeighborsWithinRadius(X, Y, ParkSpreadDistance);
                // for each TempNeighbors add AddUnique to Neighbors
                for (const auto& N : TempNeighbors)
                {
                    if (IsInBounds(N.X, N.Y) && !(N.X == TileLocationX && N.Y == TileLocationY))
                    {
                        Neighbors.AddUnique(N);
                    }
                }
            }
        }
        // for each Neighbors add the PopulationSatisfactionLayerGrid value with 50 and clamp the value to 100
        for (const auto& N : Neighbors)
        {
            // Add the PopulationSatisfactionLayerGrid on that index with 50 and Clamp the value to 100
			PolutionLayerGrid[GetIndex(N.X, N.Y)] = FMath::Clamp(PolutionLayerGrid[GetIndex(N.X, N.Y)] - 50, 0, 100);
        }
        break;
    case EBuildingTypeEnum::PoliceStation:
        // Add Police Station effects
        SecurityLayerGrid[GetIndex(TileLocationX, TileLocationY)] = FMath::Clamp(SecurityLayerGrid[GetIndex(TileLocationX, TileLocationY)] + 50, 0, 100);
        for (int32 X = TileLocationX; X <= TileLocationX + 3; ++X) {
            for (int32 Y = TileLocationY; Y <= TileLocationY + 2; ++Y) {
                TArray<FIntPoint> TempNeighbors = GetMooreNeighborsWithinRadius(X, Y, PoliceStationSpreadDistance);
                // for each TempNeighbors add AddUnique to Neighbors
                for (const auto& N : TempNeighbors)
                {
                    if (IsInBounds(N.X, N.Y) && !(N.X == TileLocationX && N.Y == TileLocationY))
                    {
                        Neighbors.AddUnique(N);
                    }
                }
            }
        }
        // for each Neighbors add the PopulationSatisfactionLayerGrid value with 50 and clamp the value to 100
        for (const auto& N : Neighbors)
        {
            // Add the PopulationSatisfactionLayerGrid on that index with 50 and Clamp the value to 100
            SecurityLayerGrid[GetIndex(N.X, N.Y)] = FMath::Clamp(SecurityLayerGrid[GetIndex(N.X, N.Y)] + 50, 0, 100);
        }
        break;
	default:
		break;
	}
}

void ACA_CityLayout::GrowDistricts(TArray<int32>& OutGrid)
{
    OutGrid = Grid;
    for (int32 y = 0; y < GridSize; ++y)
    {
        for (int32 x = 0; x < GridSize; ++x)
        {
            int32 Index = GetIndex(x, y);
            if (Grid[Index] != EMPTY)
                continue;

            TArray<FIntPoint> Neighbors = GetMooreNeighbors(x, y);
            TSet<int32> DistrictIDs;
			int32 NeighborDistrictCount = 0;

            for (const auto& N : Neighbors)
            {
                int32 NeighborValue = Grid[GetIndex(N.X, N.Y)];
                if (NeighborValue > 0)
                {
                    DistrictIDs.Add(NeighborValue);
					NeighborDistrictCount++;
                }
            }

            if ((DistrictIDs.Num() == 1 && RNG.FRand() < GrowthProb))
            {
                OutGrid[Index] = DistrictIDs.Array()[0];
            }
            else if (DistrictIDs.Num() > 1)
            {
                OutGrid[Index] = ROAD;
            }
        }
    }
}

void ACA_CityLayout::AddRoads(TArray<int32>& GridRef)
{
    TArray<int32> NewGrid = GridRef;

    for (int32 y = 0; y < GridSize; ++y)
    {
        for (int32 x = 0; x < GridSize; ++x)
        {
            int32 Index = GetIndex(x, y);
            if (NewGrid[Index] <= 0) continue;

            TArray<FIntPoint> Neighbors = GetVonNeumannNeighbors(x, y);
            TSet<int32> NeighborDistricts;

            for (const auto& N : Neighbors)
            {
                int32 Val = NewGrid[GetIndex(N.X, N.Y)];
                if (Val > 0)
                    NeighborDistricts.Add(Val);
            }

            if (NeighborDistricts.Num() > 1)
            {
                NewGrid[Index] = ROAD;
            }
        }
    }

    GridRef = NewGrid;
}

void ACA_CityLayout::PatchEmptyCells()
{
    // Find Empty cell in grid
	for (int32 y = 0; y < GridSize; ++y)
	{
		for (int32 x = 0; x < GridSize; ++x)
		{
			int32 Index = GetIndex(x, y);
			if (Grid[Index] == EMPTY)
			{
				// Check neighbors
				TArray<FIntPoint> Neighbors = GetMooreNeighbors(x, y);
				TSet<int32> NeighborDistricts;
				for (const auto& N : Neighbors)
				{
					int32 Val = Grid[GetIndex(N.X, N.Y)];
					if (Val > 0)
						NeighborDistricts.Add(Val);
				}
				// If there's only one district neighbor, fill it
				if (NeighborDistricts.Num() == 1)
				{
					Grid[Index] = NeighborDistricts.Array()[0];
				}
			}
		}
	}

    
}

TArray<FIntPoint> ACA_CityLayout::GetVonNeumannNeighborsWithinRadius(int32 StartX, int32 StartY, int32 Radius) const
{
    TArray<FIntPoint> Neighbors;

    if (Radius <= 0)
    {
        return Neighbors;
    }

    // Loop untuk seluruh offset (dx, dy) dalam radius MaxDistance
    for (int32 dx = -Radius; dx <= Radius; ++dx)
    {
        for (int32 dy = -Radius; dy <= Radius; ++dy)
        {
            // Untuk VonNeumann Neighborhood: semua cell di sekitar, termasuk diagonal
            if (FMath::Abs(dx) + FMath::Abs(dy) <= Radius)
            {
                // Cek koordinat baru (x, y)
                int32 nx = StartX + dx;
                int32 ny = StartY + dy;

                // Cek apakah di dalam grid dan bukan titik asal
                if (IsInBounds(nx, ny) && !(dx == 0 && dy == 0))
                {
                    Neighbors.AddUnique(FIntPoint(nx, ny));
                }
            }
        }
    }

    return Neighbors;
}

TArray<FIntPoint> ACA_CityLayout::GetMooreNeighborsWithinRadius(int32 StartX, int32 StartY, int32 Radius) const
{
	TArray<FIntPoint> Neighbors;

    if (Radius <= 0)
    {
        return Neighbors;
    }

	// Loop untuk seluruh offset (dx, dy) dalam radius MaxDistance
	for (int32 x = StartX - Radius; x <= StartX + Radius; ++x)
	{
		for (int32 y = StartY - Radius; y <= StartY + Radius; ++y)
		{
			// Cek apakah di dalam grid dan bukan titik asal
			if (IsInBounds(x, y) && !(x == StartX && y == StartY))
			{
				Neighbors.AddUnique(FIntPoint(x, y));
			}
		}
	}

    return Neighbors;
}


void ACA_CityLayout::InitializeRoadLayerGridValues()
{
	// Initialize the RoadLayerGrid with EMPTY values
	RoadLayerGrid.Init(EMPTY, GridSize * GridSize);
	// Set the RoadLayerGrid to the same values as the main grid
	for (int32 y = 0; y < GridSize; ++y)
	{
		for (int32 x = 0; x < GridSize; ++x)
		{
			int32 Index = GetIndex(x, y);
            if (Grid[Index] == ROAD) {
                RoadLayerGrid[Index] = Grid[Index];
            }
			
		}
	}
}

void ACA_CityLayout::InitializeDistrictLayerGridValues()
{
    // Initialize the RoadLayerGrid with EMPTY values
    DistrictLayerGrid.Init(-1, GridSize * GridSize);
    // Set the RoadLayerGrid to the same values as the main grid
    for (int32 y = 0; y < GridSize; ++y)
    {
        for (int32 x = 0; x < GridSize; ++x)
        {
            int32 Index = GetIndex(x, y);
            if (Grid[Index] > 0) {
                DistrictLayerGrid[Index] = EMPTY;
            }
            else if (Grid[Index] == ROAD) {
                DistrictLayerGrid[Index] = ROAD;
            }
        }
    }
}

void ACA_CityLayout::InitializeWaterLayerGridValues()
{
    WaterLayerGrid.Init(0, GridSize * GridSize);
    TArray <FIntPoint> WaterSeedPosition;
    WaterSeedPosition.Empty();
    int32 Attempts = 0;
    const int32 MaxAttempts = GridSize * GridSize;

    while (WaterSeedPosition.Num() < WaterSeedAmount && Attempts < MaxAttempts)
    {
        int32 X = RNG.RandRange(0, GridSize - 1);
        int32 Y = RNG.RandRange(0, GridSize - 1);
        FIntPoint Pos(X, Y);

        bool IsValid = true;
        for (const auto& Seed : WaterSeedPosition)
        {
            if (ManhattanDistance(Seed, Pos) < WaterMinSeedDistance)
            {
                IsValid = false;
                break;
            }
        }

        if (IsValid)
        {
            WaterSeedPosition.Add(Pos);
            WaterLayerGrid[GetIndex(X, Y)] = FMath::Clamp(WaterLayerGrid[GetIndex(X, Y)] + 50, 0, 100);
            TArray<FIntPoint> Neighbors = GetVonNeumannNeighborsWithinRadius(X, Y, WaterSpreadDistance);
            // for each Neighbors set the waterlayergrid to 1
            for (const auto& N : Neighbors)
            {
                // Add the WaterLayerGrid on that index with 50 and Clamp the value to 100
                WaterLayerGrid[GetIndex(N.X, N.Y)] = FMath::Clamp(WaterLayerGrid[GetIndex(N.X, N.Y)] + 50, 0, 100);
            }
        }
        Attempts++;
    }

    if (SeedPositions.Num() < NumDistricts)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to place all district seeds."));
    }
}

void ACA_CityLayout::InitializeElectricityLayerGridValues()
{
    ElectricityLayerGrid.Init(0, GridSize * GridSize);
}

void ACA_CityLayout::InitializePopulationSatisfactionLayerGridValues()
{
    PopulationSatisfactionLayerGrid.Init(10, GridSize * GridSize);
}

void ACA_CityLayout::InitializePolutionLayerGridValues()
{
    PolutionLayerGrid.Init(10, GridSize * GridSize);
}

void ACA_CityLayout::InitializePopulationDensityLayerGridValues()
{
    PopulationDensityLayerGrid.Init(0, GridSize * GridSize);
}

void ACA_CityLayout::InitializeRoadAccessibilityLayerGridValues()
{

}

void ACA_CityLayout::InitializeSecurityLayerGridValues()
{
    SecurityLayerGrid.Init(0, GridSize * GridSize);
}



void ACA_CityLayout::Simulate()
{
    TArray<int32> NewGrid;
	int32 SameCount = 0;
    while (Grid.Contains(EMPTY))
    {
        GrowDistricts(NewGrid);
        if (NewGrid == Grid)
			SameCount++;
        if (SameCount > 10) {
            // Print out the number of iterations
            UE_LOG(LogTemp, Warning, TEXT("City Layout Simulation Iterations: %d"), Iterations);
            break;
        }
        Grid = NewGrid;
        Iterations++;
    }

    AddRoads(Grid);
	PatchEmptyCells();
}

void ACA_CityLayout::VisualizeGrid()
{
    // Clear out any old instances.
    InstancedGridMesh->ClearInstances();

    // Example: create a random color for each district.
    // (Do this once per district, e.g. in BeginPlay or here.)
    
    DistrictColors.Add(ROAD, FLinearColor::Black);
    DistrictColors.Add(JUNCTION, FLinearColor::Gray);
    for (int32 DistrictID = 1; DistrictID <= NumDistricts; ++DistrictID)
    {
        DistrictColors.Add(DistrictID, FLinearColor::MakeRandomColor());
    }

    // For each cell in Grid, add an instance with color data
    for (int32 y = 0; y < GridSize; ++y)
    {
        for (int32 x = 0; x < GridSize; ++x)
        {
            const int32 CellValue = Grid[GetIndex(x, y)];
            // Skip empty cells
            if (CellValue == EMPTY)
            {
                continue;
            }

            // Build an instance transform based on x,y
            FTransform InstanceTransform;
            InstanceTransform.SetLocation(FVector(y * CellSize, x * CellSize, 0.0f));

            // Add instance
            const int32 InstanceIndex = InstancedGridMesh->AddInstance(InstanceTransform);

            // If you have a color in DistrictColors, set it
            if (DistrictColors.Contains(CellValue))
            {
                FLinearColor Color = DistrictColors[CellValue];
                // Write R,G,B to custom data
                InstancedGridMesh->SetCustomDataValue(InstanceIndex, 0, Color.R);
                InstancedGridMesh->SetCustomDataValue(InstanceIndex, 1, Color.G);
                InstancedGridMesh->SetCustomDataValue(InstanceIndex, 2, Color.B);
                InstancedGridMesh->SetCustomDataValue(InstanceIndex, 3, 1);
                InstancedGridMesh->SetCustomDataValue(InstanceIndex, 4, 0.5);
            }
        }
    }
}

void ACA_CityLayout::TrimRoads()
{
    // Create a visited array to track which cells have been walked on
    TArray<bool> Visited;
    Visited.Init(false, GridSize * GridSize);
    bool Done = false;

    // Start the random walk from any road cell
    for (int32 y = 0; y < GridSize; ++y)
    {
        for (int32 x = 0; x < GridSize; ++x)
        {
            int32 Index = GetIndex(x, y);
            if (Grid[Index] == ROAD && !Visited[Index])
            {
                // Perform a random walk starting from this road cell
                RandomWalk(x, y, Visited,-1);
                /*Done = true;
                break;*/
            }
        }
		//if (Done) break;
    }
    
    // Convert unvisited road cells into district cells
    for (int32 y = 0; y < GridSize; ++y)
    {
        for (int32 x = 0; x < GridSize; ++x)
        {
            int32 Index = GetIndex(x, y);
            if (Grid[Index] == ROAD && !Visited[Index])
            {
                // Find a neighboring district and assign its value
                TArray<FIntPoint> Neighbors = GetVonNeumannNeighbors(x, y);
                for (const auto& N : Neighbors)
                {
                    int32 NeighborIndex = GetIndex(N.X, N.Y);
                    if (Grid[NeighborIndex] > 0) // District cell
                    {
                        Grid[Index] = Grid[NeighborIndex];
                        break;
                    }
                }
            }
        }
    }
}

void ACA_CityLayout::RandomWalk(int32 X, int32 Y, TArray<bool>& Visited, int32 PrevIndex)
{
    int32 Index = GetIndex(X, Y);
	//bool hasNeighbor = false;
	//int32 neighborCount = 0;

    // Get all neighboring road cells
    TArray<FIntPoint> Neighbors;

    Visited[Index] = true;
    TSet<int32> DistrictIDs;
    for (const auto& N : GetMooreNeighbors(X, Y))
    {
        int32 NeighborIndex = GetIndex(N.X, N.Y);
        if (Grid[NeighborIndex] > 0)
        {
			DistrictIDs.Add(Grid[NeighborIndex]);
        }
    }
    // If the cell only has one District neighbor, mark it as not visited
	if (DistrictIDs.Num() == 1)
	{
		Visited[Index] = false;
	}



 //   for (const auto& N : GetVonNeumannNeighbors(X, Y))
 //   {
 //       int32 NeighborIndex = GetIndex(N.X, N.Y);
 //       if (Grid[NeighborIndex] == ROAD && NeighborIndex != PrevIndex)
 //       {
 //           hasNeighbor = true;
 //       }
	//	// Check if the neighbor is not visited and is a road
 //       if (Grid[NeighborIndex] == ROAD && !Visited[NeighborIndex])
 //       {
 //           Neighbors.Add(N);
 //       }
	//	if (Grid[NeighborIndex] == ROAD)
	//	{
	//		neighborCount++;
	//	}

 //   }
 //   //Visited[Index] = hasNeighbor;

 //   // If the cell is in the edges of the grid
 //   if (X == 0 || Y == 0 || X == GridSize - 1 || Y == GridSize - 1)
 //   {
 //       // Mark the cell as visited
 //       Visited[Index] = true;
 //       
 //   }

	//if (Neighbors.Num() == 0)
	//{
	//	return; // No neighbors to walk to
	//}
 //   else {
	//	// Mark the current cell as visited
	//	Visited[Index] = true;
 //   }

 //   /*if (neighborCount > 1)
	//{
	//	Visited[Index] = true;
	//}*/
	
    
    

    // Recursively walk to each neighbor
    /*for (const auto& N : Neighbors)
    {
        RandomWalk(N.X, N.Y, Visited, Index);
    }*/
}

void ACA_CityLayout::GetRoadJunctions()
{
	TArray<FIntPoint> Junctions;
	for (int32 y = 0; y < GridSize; ++y)
	{
		for (int32 x = 0; x < GridSize; ++x)
		{
			int32 Index = GetIndex(x, y);
			if (Grid[Index] == ROAD)
			{
				TArray<FIntPoint> Neighbors = GetMooreNeighbors(x, y);
                TSet<int32> DistrictIDs;
				for (const auto& N : Neighbors)
				{
					int32 NeighborIndex = GetIndex(N.X, N.Y);
                    if (Grid[NeighborIndex] > 0)
					{
						DistrictIDs.Add(Grid[NeighborIndex]);
					}
				}
				if (DistrictIDs.Num() > 2)
				{
					Junctions.Add(FIntPoint(x, y));
					Grid[Index] = JUNCTION; // Mark as junction
				}
			}
		}
	}
	// Do something with Junctions

    // For each Junction, add a visual representation, change the color to grey
	for (const auto& Junction : Junctions)
	{
		int32 Index = GetIndex(Junction.X, Junction.Y);
		
		// Set the color to grey
		FLinearColor Color = FLinearColor::Gray;
		InstancedGridMesh->SetCustomDataValue(Index, 0, Color.R);
		InstancedGridMesh->SetCustomDataValue(Index, 1, Color.G);
		InstancedGridMesh->SetCustomDataValue(Index, 2, Color.B);
        InstancedGridMesh->SetCustomDataValue(Index, 4, 0.5);
	}
	//print out the number of junctions
	UE_LOG(LogTemp, Warning, TEXT("Number of Junctions: %d"), Junctions.Num());
}


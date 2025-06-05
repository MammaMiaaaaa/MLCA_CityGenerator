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

	ISMBlocks = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISMBlock"));

	ISMTree = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISMTree"));

	ISMTreeV2 = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISMTree2"));

	ISMTreeV3 = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISMTree3"));

	ISMTile = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISMTile"));

	ISMGrass = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISMGrass"));

	ISMGrassV2 = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISMGrass2"));

	ISMBush = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISMBush"));

    // We want 3 floats per instance (R, G, B, A).
    InstancedGridMesh->NumCustomDataFloats = 4;
	ISMBlocks->NumCustomDataFloats = 4;
	ISMTile->NumCustomDataFloats = 2;

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

	if (bOrganizedSeedPlacement)
	{
        // Cari kombinasi rows dan cols yang hasil perkaliannya == NumSeeds, dan selisihnya minimum
        int32 BestRows = 1, BestCols = NumDistricts;
        int32 MinDiff = NumDistricts; // Selisih minimum antara rows dan cols

        for (int32 r = 1; r <= NumDistricts; ++r)
        {
            if (NumDistricts % r == 0)
            {
                int32 c = NumDistricts / r;
                if (r > 1 && c > 1 && FMath::Abs(r - c) < MinDiff)
                {
                    BestRows = r;
                    BestCols = c;
                    MinDiff = FMath::Abs(r - c);
                }
            }
        }

        bool IsFullGrid = (BestRows > 1 && BestCols > 1 && BestRows * BestCols == NumDistricts);

        if (IsFullGrid)
        {
            // Grid penuh, seed diatur dalam rows x cols
            float XSpacing = static_cast<float>(GridSize) / (BestCols + 1);
            float YSpacing = static_cast<float>(GridSize) / (BestRows + 1);

            for (int32 r = 1; r <= BestRows; ++r)
            {
                for (int32 c = 1; c <= BestCols; ++c)
                {
                    int32 X = FMath::RoundToInt(c * XSpacing);
                    int32 Y = FMath::RoundToInt(r * YSpacing);
                    SeedPositions.Add(FIntPoint(X, Y));
                    Grid[GetIndex(X, Y)] = SeedPositions.Num(); // District IDs start from 1
                }
            }
        }
        else
        {
            // Fallback: grid persegi terdekat, seed diisi sebanyak mungkin
            int32 GridDim = FMath::CeilToInt(FMath::Sqrt(static_cast<float>(NumDistricts)));
            int32 Rows = GridDim;
            int32 Cols = GridDim;
            while ((Rows - 1) * Cols >= NumDistricts) Rows--;

            float XSpacing = static_cast<float>(GridSize) / (Cols + 1);
            float YSpacing = static_cast<float>(GridSize) / (Rows + 1);

            int32 SeedCount = 0;
            for (int32 r = 1; r <= Rows && SeedCount < NumDistricts; ++r)
            {
                for (int32 c = 1; c <= Cols && SeedCount < NumDistricts; ++c)
                {
                    int32 X = FMath::RoundToInt(c * XSpacing);
                    int32 Y = FMath::RoundToInt(r * YSpacing);
                    SeedPositions.Add(FIntPoint(X, Y));
                    Grid[GetIndex(X, Y)] = SeedPositions.Num(); // District IDs start from 1
                    ++SeedCount;
                }
            }
        }
	}
	else
	{
		// Random seed placement
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
	}
    

    if (SeedPositions.Num() < NumDistricts)
    {
        //UE_LOG(LogTemp, Error, TEXT("Failed to place all district seeds."));
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
        //UE_LOG(LogTemp, Warning, TEXT("InstancedGridMesh is null in AddRoadsToArray."));
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
            //UE_LOG(LogTemp, Warning, TEXT("Invalid index %d in RoadIndexArray"), Index);
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

void ACA_CityLayout::UpdateISMToSpecificLayer(ELayerEnum LayerEnum)
{
    FLinearColor GrayColor = FLinearColor::Gray;
    FLinearColor BlackColor = FLinearColor::Black;
    FLinearColor GreenColor = FLinearColor::Green;
    FLinearColor BlueColor = FLinearColor::Blue;
    FLinearColor YellowColor = FLinearColor::Yellow;
    FLinearColor RedColor = FLinearColor::Red;
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
                    InstancedGridMesh->SetCustomDataValue(Index, 4, 0.5);
				}
				else {
					InstancedGridMesh->SetCustomDataValue(Index, 0, GrayColor.R);
					InstancedGridMesh->SetCustomDataValue(Index, 1, GrayColor.G);
					InstancedGridMesh->SetCustomDataValue(Index, 2, GrayColor.B);
                    InstancedGridMesh->SetCustomDataValue(Index, 3, 1);
                    InstancedGridMesh->SetCustomDataValue(Index, 4, 0.5);
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
        // for loop through the PolutionLayerGrid and set the material color according to the value
        for (int32 y = 0; y < GridSize; ++y)
        {
            for (int32 x = 0; x < GridSize; ++x)
            {
                int32 Index = GetIndex(x, y);
                if (RoadAccessibilityLayerGrid[Index] > 0)
                {
                    InstancedGridMesh->SetCustomDataValue(Index, 0, RedColor.R);
                    InstancedGridMesh->SetCustomDataValue(Index, 1, YellowColor.G);
                    InstancedGridMesh->SetCustomDataValue(Index, 2, YellowColor.B);
                    InstancedGridMesh->SetCustomDataValue(Index, 3, static_cast<float>(RoadAccessibilityLayerGrid[Index]) / 100.0);
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
        //UE_LOG(LogTemp, Warning, TEXT("UpdateLayerValues: Unknown LayerEnum value!"));
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

void ACA_CityLayout::CalculateDistrictType()
{
	// Loop through the DistrictArray
    if (bUseMLCAValues) 
    {
        for (int32 i = 0; i < DistrictArray.Num(); ++i)
        {

            float WaterValue = 0;
            float ElectricityValue = 0;
            float PopulationSatisfactionValue = 0;
            float PolutionValue = 0;
            float PopulationDensityValue = 0;
            float RoadAccessibilityValue = 0;
            float SecurityValue = 0;

            bool bIsEnoughWater = false;
            bool bIsEnoughElectricity = false;
            bool bIsEnoughPopulationSatisfaction = false;
            bool bIsEnoughPolution = false;
            bool bIsEnoughPopulationDensity = false;
            bool bIsEnoughRoadAccessibility = false;
            bool bIsEnoughSecurity = false;

            float MaxValue = DistrictArray[i].DistrictCellIndex.Num() * 100;

            // Loop for each cell index in the DistrictCellIndex
            for (int32 j = 0; j < DistrictArray[i].DistrictCellIndex.Num(); ++j)
            {
                WaterValue += WaterLayerGrid[DistrictArray[i].DistrictCellIndex[j]];
                ElectricityValue += ElectricityLayerGrid[DistrictArray[i].DistrictCellIndex[j]];
                PopulationSatisfactionValue += PopulationSatisfactionLayerGrid[DistrictArray[i].DistrictCellIndex[j]];
                PolutionValue += PolutionLayerGrid[DistrictArray[i].DistrictCellIndex[j]];
                PopulationDensityValue += PopulationDensityLayerGrid[DistrictArray[i].DistrictCellIndex[j]];
                RoadAccessibilityValue += RoadAccessibilityLayerGrid[DistrictArray[i].DistrictCellIndex[j]];
                SecurityValue += SecurityLayerGrid[DistrictArray[i].DistrictCellIndex[j]];

                if (WaterValue >= MaxValue * WaterThreshold / 100) {
                    bIsEnoughWater = true;
                }
                if (ElectricityValue >= MaxValue * ElectricityThreshold / 100) {
                    bIsEnoughElectricity = true;
                }
                if (PopulationSatisfactionValue >= MaxValue * PopulationSatisfactionThreshold / 100) {
                    bIsEnoughPopulationSatisfaction = true;
                }
                if (PolutionValue < MaxValue * PolutionThreshold / 100) {
                    bIsEnoughPolution = true;
                }
                if (PopulationDensityValue >= MaxValue * PopulationDensityThreshold / 100) {
                    bIsEnoughPopulationDensity = true;
                }
                if (RoadAccessibilityValue >= MaxValue * RoadAccessibilityThreshold / 100) {
                    bIsEnoughRoadAccessibility = true;
                }
                if (SecurityValue >= MaxValue * SecurityThreshold / 100) {
                    bIsEnoughSecurity = true;
                }
            }
            DistrictArray[i].WaterAvailValue = (WaterValue / MaxValue);
            DistrictArray[i].ElectricityAvailValue = (ElectricityValue / MaxValue);
            DistrictArray[i].PopulationSatisfactionValue = (PopulationSatisfactionValue / MaxValue);
            DistrictArray[i].PolutionValue = (PolutionValue / MaxValue);
            DistrictArray[i].PopulationDensityValue = (PopulationDensityValue / MaxValue);
            DistrictArray[i].RoadAccessibilityValue = (RoadAccessibilityValue / MaxValue);
            DistrictArray[i].SecurityValue = (SecurityValue / MaxValue);

            if (bIsEnoughWater && bIsEnoughElectricity && bIsEnoughPopulationSatisfaction && bIsEnoughPolution && bIsEnoughSecurity)
            {
                DistrictArray[i].AvailableDistrictType.AddUnique(RESIDENTIAL);
            }
            if (bIsEnoughWater && bIsEnoughElectricity && bIsEnoughPopulationDensity && bIsEnoughRoadAccessibility && bIsEnoughSecurity)
            {
                DistrictArray[i].AvailableDistrictType.AddUnique(COMMERCIAL);
            }
            if (bIsEnoughWater && bIsEnoughElectricity && bIsEnoughRoadAccessibility && bIsEnoughPolution)
            {
                DistrictArray[i].AvailableDistrictType.AddUnique(INDUSTRIAL);
            }
            // Randomly assign the DistrictType with AvailableDistrictType
            if (DistrictArray[i].AvailableDistrictType.Num() > 0)
            {
                int32 RandomIndex = FMath::RandRange(0, DistrictArray[i].AvailableDistrictType.Num() - 1);
                DistrictArray[i].DistrictType = DistrictArray[i].AvailableDistrictType[RandomIndex];
            }
            else
            {
                DistrictArray[i].DistrictType = 0;
            }
            // Set the DistrictLayerGrid to the DistrictType
            for (int32 j = 0; j < DistrictArray[i].DistrictCellIndex.Num(); ++j)
            {
                DistrictLayerGrid[DistrictArray[i].DistrictCellIndex[j]] = DistrictArray[i].DistrictType;
            }
        }
    }
    // Use District Weight to Choose District Type
    else {
		int32 TotalDistrictWeight = ResidentialDistrictWeight + CommercialDistrictWeight + IndustrialDistrictWeight;
		for (int32 i = 0; i < DistrictArray.Num(); ++i)
		{

			// Get the DistrictType from the DistrictWeight
			int32 DistrictType = FMath::RandRange(1, TotalDistrictWeight);
            if (DistrictType <= ResidentialDistrictWeight) {
				DistrictArray[i].DistrictType = RESIDENTIAL;
            }
			else if (DistrictType <= ResidentialDistrictWeight + CommercialDistrictWeight) {
				DistrictArray[i].DistrictType = COMMERCIAL;
			}
			else if (DistrictType <= ResidentialDistrictWeight + CommercialDistrictWeight + IndustrialDistrictWeight) {
				DistrictArray[i].DistrictType = INDUSTRIAL;
			}
			else {
				DistrictArray[i].DistrictType = EMPTY;
			}
			// Set the DistrictLayerGrid to the DistrictType
			for (int32 j = 0; j < DistrictArray[i].DistrictCellIndex.Num(); ++j)
			{
				DistrictLayerGrid[DistrictArray[i].DistrictCellIndex[j]] = DistrictArray[i].DistrictType;
			}
		}
    }
}

void ACA_CityLayout::GetAllDistricts()
{
	// Loop through the grid and find all districts
    for (int32 y = 0; y < GridSize; ++y) {
		for (int32 x = 0; x < GridSize; ++x) {
			int32 Index = GetIndex(x, y);
			if (Grid[Index] > 0) {
                if (DistrictArray.Num() == 0)
                {
                    // If the DistrictArray is empty, create a new district
                    FDistrictStruct NewDistrict;
                    NewDistrict.DistrictID = Grid[Index];
                    NewDistrict.DistrictType = 0;
                    NewDistrict.DistrictCellIndex.Add(Index);

                    DistrictArray.Add(NewDistrict);
                }
                else {
                    // Check if the DistrictArray have DistrictID same with the DistrictIDs.Array()[0]

                    bool bFound = false;

                    for (int32 i = 0; i < DistrictArray.Num(); ++i)
                    {
                        if (DistrictArray[i].DistrictID == Grid[Index])
                        {
                            // Add the index to the DistrictArray
                            DistrictArray[i].DistrictCellIndex.Add(Index);
                            bFound = true;
                            break;
                        }
                    }
                    if (!bFound)
                    {
                        // If not found, create a new district
                        FDistrictStruct NewDistrict;
                        NewDistrict.DistrictID = Grid[Index];
                        NewDistrict.DistrictType = 0;
                        NewDistrict.DistrictCellIndex.Add(Index);
                        DistrictArray.Add(NewDistrict);
                    }
                }
			}
		}
    }


    
}

void ACA_CityLayout::PartitionGridBSP(TArray<int32> DistrictIndex, int DistrictType)
{
    // Siapkan output
    int32 nextID = 0;
    minBlockValue = DistrictIndex[0];
	maxBlockValue = DistrictIndex.Last();
	int32 X = GetMinX(DistrictIndex);
    int32 Y = GetMinY(DistrictIndex);
	int32 Width = GetMaxWidth(DistrictIndex, X);
	int32 Height = GetMaxHeight(DistrictIndex, Y);
    
    // Panggil rekursif atas seluruh grid [0,0,GridSize,GridSize]
    DoBSP_Grid(X, Y, Width, Height, nextID, DistrictIndex, DistrictType);
}

void ACA_CityLayout::FloorPlanAllDistricts()
{
	// Loop through all the districts
	for (int32 i = 0; i < DistrictArray.Num(); ++i)
	{
        //UE_LOG(LogTemp, Warning, TEXT("District %d: "), i);
        //UE_LOG(LogTemp, Warning, TEXT("=============="));
        CurrentDistrict = i;
		PartitionGridBSP(DistrictArray[i].DistrictCellIndex, DistrictArray[i].DistrictType);
		
	}
    // Print CellCount Variable
    //UE_LOG(LogTemp, Warning, TEXT("CellCount: %d"),CellCount);
    
}

void ACA_CityLayout::VisualizeTheBlocks()
{
	for (int32 i = 0; i < GridSize; ++i)
	{
		for (int32 j = 0; j < GridSize; ++j)
		{
			int32 Index = GetIndex(j, i);
			if (Grid[Index] > 0)
			{
				FTransform Transform;
				InstancedGridMesh->GetInstanceTransform(Index, Transform, true);
				ISMBlocks->AddInstance(Transform, true);
				ISMBlocks->SetCustomDataValue(Index, 0, 0);
				ISMBlocks->SetCustomDataValue(Index, 1, 0);
				ISMBlocks->SetCustomDataValue(Index, 2, 0);
				ISMBlocks->SetCustomDataValue(Index, 3, 1);
				ISMBlocks->SetCustomDataValue(Index, 4, 0.5);
			}
		}
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

            if ((DistrictIDs.Num() == 1 && RNG.FRand() <= GrowthProb))
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

bool ACA_CityLayout::HasNeighboringRoad(int32 StartX, int32 StartY, int32 Radius) const
{
    // Loop untuk seluruh offset (dx, dy) dalam radius MaxDistance
    for (int32 x = StartX - Radius; x <= StartX + Radius; ++x)
    {
        for (int32 y = StartY - Radius; y <= StartY + Radius; ++y)
        {
            // Cek apakah di dalam grid dan bukan titik asal
            if (IsInBounds(x, y) && !(x == StartX && y == StartY))
            {
				if (Grid[GetIndex(x, y)] == ROAD)
				{
					return true; // Ada jalan di sekitar titik ini
				}
            }
        }
    }
    return false;
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

void ACA_CityLayout::GetAllLargestRectanglesForDistricts() {
    // for each loop DistrictArray
	for (int32 i = 0; i < DistrictArray.Num(); ++i)
	{
		// for each loop BlockCellArray
		for (int32 j = 0; j < DistrictArray[i].BlockCellArray.Num(); ++j)
		{
			FindLargestRectangle(DistrictArray[i].BlockCellArray[j].BlockArray, DistrictArray[i].BlockCellArray[j].SizeX, DistrictArray[i].BlockCellArray[j].SizeY, DistrictArray[i].DistrictType);

		}
	}

}

void ACA_CityLayout::SetBuildingLayerGridValues()
{
	BuildingLayerArray.Init(0, GridSize * GridSize);
	for (FDistrictStruct& District : DistrictArray)
	{
		for (FBlockCellStruct& BlockCell : District.BlockCellArray)
		{
            // Get Building Edge Cell
			if (BlockCell.BlockArray.Num() == 0) continue;
			int32 MinRow = BlockCell.BlockArray[0] / GridSize;
			int32 MinCol = BlockCell.BlockArray[0] % GridSize;
			int32 MaxRow = BlockCell.BlockArray.Last() / GridSize;
			int32 MaxCol = BlockCell.BlockArray.Last() % GridSize;
			for (int32 BlockIndex : BlockCell.BlockArray)
			{
				// Set the BuildingLayerGrid to the BlockIndex
				BuildingLayerArray[BlockIndex] = District.DistrictType;
                if (District.DistrictType == RESIDENTIAL)
                {
					// BlockIndex is in the same row or column as the edge cells
                    //if (BlockIndex / GridSize == MinRow || BlockIndex / GridSize == MaxRow || BlockIndex % GridSize == MinCol || BlockIndex % GridSize == MaxCol) {
                        // Set the TileLayerArray to YARD
						TileLayerArray[BlockIndex] = YARD;
						FTransform InstanceTransform;
                        InstancedGridMesh->GetInstanceTransform(BlockIndex, InstanceTransform, true);
						InstanceTransform.AddToTranslation(FVector(0, 0, 15)); // Adjust height for visibility
						//ISMTile->AddInstance(InstanceTransform, true);
                    //}
					// Print the BlockIndex and the DistrictType
						UE_LOG(LogTemp, Warning, TEXT("BlockIndex: %d, DistrictType: %d"), BlockIndex, District.DistrictType);
                }
			}
		}
	}
}

void ACA_CityLayout::SetTreeLayerGridValues()
{
	TreeLayerArray.Init(0, GridSize * GridSize);
	int32 Index = 0;
	int32 Counter = 0;
	// for each loop Grid
    /*for (int32 GridIndex : Grid)
    {
        if (GridIndex > 0 && FMath::RandRange(1, 10) > 9)
        {
			TreeLayerArray.Add(Index);
        }
		Index++;
        
		
    }*/
	// for each loop DistrictArray
	for (FDistrictStruct& District : DistrictArray)
	{
		for (FBlockCellStruct BlockCell: District.BlockCellArray)
		{
			for (int32 BlockIndex : BlockCell.BlockArray)
			{
                TreeLayerArray[BlockIndex] = -1;
			}
		}
	}
	// Get transform from InstancedGridMesh

	for (int32 TreeIndex = 0; TreeIndex < GridSize * GridSize; ++TreeIndex)
	{
		FTransform InstanceTransform;
		// Get the transform from the InstancedGridMesh
		InstancedGridMesh->GetInstanceTransform(TreeIndex, InstanceTransform, true);
        
        // Add Random Vector to InstanceTransform Location
		float OffsetAmount = static_cast<float>(GridSize) / 2.0f;
        FVector RandomOffset = FVector(FMath::RandRange(-OffsetAmount, OffsetAmount), FMath::RandRange(-OffsetAmount, OffsetAmount), 0.0f);
		InstanceTransform.AddToTranslation(RandomOffset);
		InstanceTransform.SetRotation(FQuat::MakeFromEuler(FVector(0, 0, FMath::RandRange(0, 360)))); // Random rotation around Z-axis

        if (Grid[TreeIndex] < 0)
			TreeLayerArray[TreeIndex] = -1; // Skip if not valid for tree placement

		if (DistrictLayerGrid[TreeIndex] == COMMERCIAL)
			TreeLayerArray[TreeIndex] = -1; // Skip if not valid for tree placement

		if (DistrictLayerGrid[TreeIndex] == INDUSTRIAL && !HasNeighboringRoad(TreeIndex % GridSize, TreeIndex / GridSize, 1))
			TreeLayerArray[TreeIndex] = -1; // Skip if not valid for tree placement

		if (TreeLayerArray[TreeIndex] == -1) continue; // Skip if not valid for tree placement
        // Randomize the InstanceStaticMesh Variant
		int32 RandomVariant = FMath::RandRange(0, 100);
        if (RandomVariant < 5) {
			TreeLayerArray[TreeIndex] = 1; // 5% chance for ISMTree
			ISMTree->AddInstance(InstanceTransform, true);
		}
		else if (RandomVariant < 10) {
            TreeLayerArray[TreeIndex] = 2; // 5% chance for ISMTreeV2
			ISMTreeV2->AddInstance(InstanceTransform, true);
		}
		else if (RandomVariant < 15) {
            TreeLayerArray[TreeIndex] = 3; // 5% chance for ISMTreeV3
			ISMTreeV3->AddInstance(InstanceTransform, true);
		}
        else if (RandomVariant < 35 && DistrictLayerGrid[TreeIndex] != INDUSTRIAL) {
            TreeLayerArray[TreeIndex] = 4; // 20% chance for ISMGrass
			int32 RandomHeight = FMath::RandRange(0, 30);
			InstanceTransform.AddToTranslation(FVector(0, 0, RandomHeight)); // Adjust height for visibility
			ISMGrass->AddInstance(InstanceTransform, true);
        }
        else if (RandomVariant < 55 && DistrictLayerGrid[TreeIndex] != INDUSTRIAL) {
            TreeLayerArray[TreeIndex] = 5; // 20% chance for ISMGrassV2
			float RandomSize = FMath::RandRange(1.5f, 2.5f);
			InstanceTransform.SetScale3D(FVector(RandomSize, RandomSize, RandomSize)); // Random scale for grass
			ISMGrassV2->AddInstance(InstanceTransform, true);
        }
        else if (RandomVariant < 65 && DistrictLayerGrid[TreeIndex] != INDUSTRIAL) {
            TreeLayerArray[TreeIndex] = 6; // 10% chance for ISMBush
			ISMBush->AddInstance(InstanceTransform, true);
        }
        
		
		
		
	}
}

void ACA_CityLayout::CalculateTileLayerGridValues()
{
	// Initialize the TileLayerGrid with EMPTY values
	TileLayerArray.Init(EMPTY, GridSize * GridSize);
	// Set the TileLayerGrid to the same values as the main grid
	for (int32 y = 0; y < GridSize; ++y)
	{
		for (int32 x = 0; x < GridSize; ++x)
		{
            int32 Index = GetIndex(x, y);
            FTransform InstanceTransform;
            InstancedGridMesh->GetInstanceTransform(Index, InstanceTransform, true);
            int32 ISMIndex = ISMTile->AddInstance(InstanceTransform, true);
			
			//if (Grid[Index] > 0) {
   //             TileLayerArray[Index] = Grid[Index];
   //             ISMTile->SetCustomDataValue(ISMIndex, 0, 1);
			//	//ISMTile->SetCustomPrimitiveDataFloat(Index, 0, 1.0f); // Set the custom primitive data to 1.0f for visibility
			//}
			//else if (Grid[Index] == ROAD) {
   //             TileLayerArray[Index] = ROAD;
			//	ISMTile->SetCustomDataValue(ISMIndex, 0, 0);
			//}

            ISMTile->SetCustomDataValue(ISMIndex, 0, DistrictLayerGrid[Index]);
			
		}
	}
}

void ACA_CityLayout::AddRoadWidth()
{
	TArray<int32> GridCopy = Grid;
	// Loop through the RoadLayerGrid and set the RoadWidth to 1
	for (int32 y = 0; y < GridSize; ++y)
	{
		for (int32 x = 0; x < GridSize; ++x)
		{
			int32 Index = GetIndex(x, y);
			if (HasNeighboringRoad(x, y, AdditionalRoadWidth))
			{
				GridCopy[Index] = ROAD;
			}
			
		}
	}
	Grid = GridCopy;
}

void ACA_CityLayout::CalculateRoadType()
{
	// Initialize the RoadTypeLayerGrid with EMPTY values
	RoadLayerGrid.Init(EMPTY, GridSize * GridSize);
	// Set the RoadTypeLayerGrid to the same values as the main grid
	for (int32 y = 0; y < GridSize; ++y)
	{
		for (int32 x = 0; x < GridSize; ++x)
		{
			int32 Index = GetIndex(x, y);
			if (Grid[Index] == ROAD) {
				RoadLayerGrid[Index] = GetRoadType(Index);
				ISMTile->SetCustomDataValue(Index, 1, RoadLayerGrid[Index], true);
			}
		}
	}
}

void ACA_CityLayout::SetBuildingDirection()
{
    // for each loop DistrictArray
    for (int32 i = 0; i < DistrictArray.Num(); ++i)
    {
        // for each loop BlockCellArray
        for (int32 j = 0; j < DistrictArray[i].BlockCellArray.Num(); ++j)
        {
			// for each loop BlockArray
			int32 HighestRoadAccessibilityValue = 0;
			int32 HighestRoadAccessibilityIndex = -1;
            for (int32 k = 0; k < DistrictArray[i].BlockCellArray[j].BlockArray.Num(); ++k)
            {
                // Get Highest RoadAccessibilityLayerGrid value in the BlockArray
                int32 BlockIndex = DistrictArray[i].BlockCellArray[j].BlockArray[k];
                if (RoadAccessibilityLayerGrid[BlockIndex] > HighestRoadAccessibilityValue) {
                    HighestRoadAccessibilityValue = RoadAccessibilityLayerGrid[BlockIndex];
					HighestRoadAccessibilityIndex = BlockIndex;
                }
            }
            // Get the center X and Y of BlockCell Array
			/*int32 CenterX = (DistrictArray[i].BlockCellArray[j].BlockArray[DistrictArray[i].BlockCellArray[j].BlockArray.Num() - 1]) % GridSize - (DistrictArray[i].BlockCellArray[j].SizeX / 2);
			int32 CenterY = (DistrictArray[i].BlockCellArray[j].BlockArray[DistrictArray[i].BlockCellArray[j].BlockArray.Num() - 1]) / GridSize - (DistrictArray[i].BlockCellArray[j].SizeY / 2);*/
            if (DistrictArray[i].BlockCellArray[j].BlockArray.Num() > 0)
            {
                int32 BotLeftX = DistrictArray[i].BlockCellArray[j].BlockArray[0] % GridSize;
                int32 BotLeftY = DistrictArray[i].BlockCellArray[j].BlockArray[0] / GridSize;
                int32 TopRightX = DistrictArray[i].BlockCellArray[j].BlockArray.Last() % GridSize;
                int32 TopRightY = DistrictArray[i].BlockCellArray[j].BlockArray.Last() / GridSize;

                int32 HighestRoadAccessibilityX = HighestRoadAccessibilityIndex % GridSize;
                int32 HighestRoadAccessibilityY = HighestRoadAccessibilityIndex / GridSize;

                // Calculate the direction from CenterX, CenterY to HighestRoadAccessibilityX, HighestRoadAccessibilityY
                // Check if the HighestRoadAccessibilityX is less than CenterX 
                int32 x = 0;
                while (true) {
                    // if the HighestRoadAccessibilityX is the same coloumn Bot LeftX so the direction is LEFT
                    if (HighestRoadAccessibilityX == BotLeftX + x) {
                        DistrictArray[i].BlockCellArray[j].BuildingDirection = LEFT;
						UE_LOG(LogTemp, Warning, TEXT("BlockCell %d in District %d BuildingDirection: LEFT"), j, i);
                        break;
                    }
                    // if the HighestRoadAccessibilityX is the same coloumn Top RightX so the direction is RIGHT
                    else if (HighestRoadAccessibilityX == TopRightX - x) {
                        DistrictArray[i].BlockCellArray[j].BuildingDirection = RIGHT;
						UE_LOG(LogTemp, Warning, TEXT("BlockCell %d in District %d BuildingDirection: RIGHT"), j, i);
                        break;
                    }
                    // if the HighestRoadAccessibilityY is the same row Bot LeftY so the direction is BOTTOM
                    else if (HighestRoadAccessibilityY == BotLeftY + x) {
                        DistrictArray[i].BlockCellArray[j].BuildingDirection = BOTTOM;
						UE_LOG(LogTemp, Warning, TEXT("BlockCell %d in District %d BuildingDirection: BOTTOM"), j, i);
                        break;
                    }
                    // if the HighestRoadAccessibilityY is the same row Top RightY so the direction is TOP
                    else if (HighestRoadAccessibilityY == TopRightY - x) {
                        DistrictArray[i].BlockCellArray[j].BuildingDirection = TOP;
						UE_LOG(LogTemp, Warning, TEXT("BlockCell %d in District %d BuildingDirection: TOP"), j, i);
                        break;
                    }
                    x++;
                    if (x > 100) {
                        //UE_LOG(LogTemp, Warning, TEXT("Failed to set BuildingDirection for BlockCell %d in District %d"), j, i);
                        //DistrictArray[i].BlockCellArray[j].BuildingDirection = NONE;
                        break;
                    }
                }
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
        //UE_LOG(LogTemp, Error, TEXT("Failed to place all district seeds."));
    }
}

void ACA_CityLayout::InitializeElectricityLayerGridValues()
{
    ElectricityLayerGrid.Init(10, GridSize * GridSize);
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
    //RoadAccessibilityLayerGrid.Init(10, GridSize * GridSize);
	// for loop each cell in the grid
	RoadAccessibilityLayerGrid.Init(0, GridSize * GridSize);
	for (int32 y = 0; y < GridSize; ++y)
	{
		for (int32 x = 0; x < GridSize; ++x)
		{
			int32 Index = GetIndex(x, y);
			if (Grid[Index] == ROAD) {
				RoadAccessibilityLayerGrid[Index] = 100;
			}
			int32 RoadAccessibilityValue = 100 - 20 * (GetMinDistanceToRoad(x, y) - 1);
			if (RoadAccessibilityValue < 0) {
				RoadAccessibilityValue = 0;
			}
			else if (RoadAccessibilityValue > 100) {
				RoadAccessibilityValue = 0;
			}
			RoadAccessibilityLayerGrid[Index] = RoadAccessibilityValue;
		}
	}
}

void ACA_CityLayout::InitializeSecurityLayerGridValues()
{
    SecurityLayerGrid.Init(10, GridSize * GridSize);
}

void ACA_CityLayout::DoBSP_Grid(int32 X, int32 Y, int32 Width, int32 Height, int32& NextID, TArray<int32>& BlockIndex, int DistrictType)
{
	// Tentukan ukuran minimum untuk split
	int32 minXSize = 2;
	int32 minYSize = 2;
	int32 maxXSize = 10;
	int32 maxYSize = 10;
    switch (DistrictType)
    {
    case 0:
        minXSize = 2;
        minYSize = 2;
        maxXSize = 10;
        maxYSize = 10;
        break;
    case 1:
        minXSize = minXSizeResidental;
        minYSize = minYSizeResidental;
        maxXSize = maxXSizeResidental;
        maxYSize = maxYSizeResidental;
        break;
    case 2:
        minXSize = minXSizeCommercial;
        minYSize = minYSizeCommercial;
        maxXSize = maxXSizeCommercial;
        maxYSize = maxYSizeCommercial;
        break;
    case 3:
        minXSize = minXSizeIndustrial;
        minYSize = minYSizeIndustrial;
        maxXSize = maxXSizeIndustrial;
        maxYSize = maxYSizeIndustrial;
        break;
    }
        
    // Cek apakah region perlu di-split lagi
    bool canSplitX = Width > maxXSize && Width >= 2 * minXSize;
    bool canSplitY = Height > maxYSize && Height >= 2 * minYSize;

    TArray<int32> LeftPartition;
    TArray<int32> RightPartition;

    /*UE_LOG(LogTemp, Warning, TEXT("==========="));
	UE_LOG(LogTemp, Warning, TEXT("X: %d"), X);
    UE_LOG(LogTemp, Warning, TEXT("Y: %d"), Y);
    UE_LOG(LogTemp, Warning, TEXT("==========="));*/


    if (!canSplitX && !canSplitY)
    {
        if (BlockIndex.Num() > 0)
        {
            // Buat BlockArrayCell baru dengan ID sekarang
            FBlockCellStruct NewBlockCell;
            int32 currentID = NextID++;
            NewBlockCell.BlockID = currentID;
			bool bHasRoadAccess = false;
            // Loop untuk menambahkan BlockIndex ke dalam NewBlockCell BlockArray
            for (int32 i = 0; i < BlockIndex.Num(); ++i)
            {
                // Print out the BlockIndex[i]
                //UE_LOG(LogTemp, Warning, TEXT("BlockIndex[%d]: %d"), i, BlockIndex[i]);

                NewBlockCell.BlockArray.Add(BlockIndex[i]);
                CellCount++;
				if (RoadAccessibilityLayerGrid[BlockIndex[i]] > 0)
				{
					bHasRoadAccess = true;
				}
            }
            //UE_LOG(LogTemp, Warning, TEXT("=========================="));
            if (bHasRoadAccess)
            DistrictArray[CurrentDistrict].BlockCellArray.Add(NewBlockCell);
            
        }
        return;
    }

    // Tentukan orientasi split: berdasarkan ukuran
    bool splitVert = false;
    if (canSplitX && canSplitY)
    {
        // Pilih berdasarkan sisi terpanjang
        splitVert = (Width > Height);
    }
    else if (canSplitX)
    {
        splitVert = true;
    }
    // else splitVert tetap false (split horizontal)

    if (splitVert)
    {
        // Vertical split: cari splitX di [MinX, W-MinX]
        int32 SplitAt = FMath::RandRange(minXSize, Width - minXSize);

		//UE_LOG(LogTemp, Warning, TEXT("SplitVertAt: %d"), SplitAt);

		
		// loop to add the partition to the left
        for (int32 i = 0; i < Height; ++i) {
            for (int32 j = 0; j < SplitAt; ++j) {
                if (BlockIndex.Contains((j + X) + ((i + Y) * GridSize)) ) {
					LeftPartition.Add((j + X) + ((i + Y) * GridSize));
					//UE_LOG(LogTemp, Warning, TEXT("LeftPartition: %d"), (j + X) + ((i + Y) * GridSize));
                }
                else {
					//UE_LOG(LogTemp, Warning, TEXT("Else Left"));
                }
            }
        }
        for (int32 i = 0; i < Height; ++i) {
            for (int32 j = SplitAt; j < Width; ++j) {
                if (BlockIndex.Contains((j + X) + ((i + Y) * GridSize))) {
                    RightPartition.Add((j + X) + ((i + Y) * GridSize));
					//UE_LOG(LogTemp, Warning, TEXT("RightPartition: %d"), (j + X) + ((i + Y) * GridSize));
                }
                else {
                    //UE_LOG(LogTemp, Warning, TEXT("Else Right"));
                }
            }
        }
		X = GetMinX(LeftPartition);
		Y = GetMinY(LeftPartition);

        // Region kiri
        if (LeftPartition.Num() > 0)
            DoBSP_Grid(X, Y, SplitAt, GetMaxHeight(LeftPartition,Y), NextID, LeftPartition, DistrictType);
        // Region kanan

        //X = GetMinX(RightPartition);
        Y = GetMinY(RightPartition);
        if (RightPartition.Num() > 0)
            DoBSP_Grid(X + SplitAt, Y, Width - SplitAt, GetMaxHeight(RightPartition, Y), NextID, RightPartition, DistrictType);
    }
    else
    {
        // Horizontal split: cari splitY di [MinY, H-MinY]
        int32 SplitAt = FMath::RandRange(minYSize, Height - minYSize);

        //UE_LOG(LogTemp, Warning, TEXT("SplitHorAt: %d"), SplitAt);

        TArray<int32> LowerPartition;
        TArray<int32> UpperPartition;
		// loop to add the Lower partition
        for (int32 i = 0; i < SplitAt; ++i) {
            for (int32 j = 0; j < Width; ++j) {
                if (BlockIndex.Contains((j + X) + ((i + Y) * GridSize))) {
                    LowerPartition.Add((j + X) + ((i + Y) * GridSize));
					//UE_LOG(LogTemp, Warning, TEXT("LowerPartition: %d"), (j + X) + ((i + Y) * GridSize));
                }
                else {
                    //UE_LOG(LogTemp, Warning, TEXT("Else Lower"));
                }
            }
        }
		// loop to add the Upper partition
        for (int32 i = SplitAt; i < Height; ++i) {
            for (int32 j = 0; j < Width; ++j) {
                if (BlockIndex.Contains((j + X) + ((i + Y) * GridSize))) {
                    UpperPartition.Add((j + X) + ((i + Y) * GridSize));
                    //UE_LOG(LogTemp, Warning, TEXT("UpperPartition: %d"), (j + X) + ((i + Y) * GridSize));
                }
                else {
                    //UE_LOG(LogTemp, Warning, TEXT("Else Upper"));
                }
            }
        }
        X = GetMinX(UpperPartition);
        //Y = GetMinY(UpperPartition);
        // Region atas
        if (UpperPartition.Num() > 0)
            DoBSP_Grid(X, Y + SplitAt, GetMaxWidth(UpperPartition,X), Height - SplitAt, NextID, UpperPartition, DistrictType);
        // Region bawah
        X = GetMinX(LowerPartition);
        Y = GetMinY(LowerPartition);
        if (LowerPartition.Num() > 0)
            DoBSP_Grid(X, Y, GetMaxWidth(LowerPartition, X), SplitAt, NextID, LowerPartition, DistrictType);
    }
}

int32 ACA_CityLayout::GetMaxHeight(TArray<int32>& GridArray, int32 Y)
{
	if (GridArray.Num() == 0)
	{
		Y = 0;
        //UE_LOG(LogTemp, Warning, TEXT("Array is Empty"));
		return 0;
	}
    Y = GridArray[0] / GridSize;
	//UE_LOG(LogTemp, Warning, TEXT("Y: %d"), Y);
	//UE_LOG(LogTemp, Warning, TEXT("Height: %d"), (GridArray[GridArray.Num() - 1] / GridSize) - (GridArray[0] / GridSize) + 1);
	return (GridArray.Last()/GridSize) - (GridArray[0]/GridSize) + 1;
}

int32 ACA_CityLayout::GetMaxWidth(TArray<int32>& GridArray, int32 X)
{
    int32 MaxValue = 0;
	int32 MinValue = 1000000000;
	for (int32 i = 0; i < GridArray.Num(); i++)
	{
		if (GridArray[i] % GridSize > MaxValue)
		{
			MaxValue = GridArray[i] % GridSize;
		}
		if (GridArray[i] % GridSize < MinValue)
		{
			MinValue = GridArray[i] % GridSize;
		}
	}
	X = MinValue;
	//UE_LOG(LogTemp, Warning, TEXT("X: %d"), X);
	//UE_LOG(LogTemp, Warning, TEXT("Width: %d"), MaxValue - MinValue + 1);
	return MaxValue - MinValue + 1;
}

int32 ACA_CityLayout::GetMinX(TArray<int32>& GridArray)
{
    if (GridArray.Num() == 0)
    {
        return 0;
    }
    int32 MinValue = 1000000000;
    for (int32 i = 0; i < GridArray.Num(); i++)
    {
        if (GridArray[i] % GridSize < MinValue)
        {
            MinValue = GridArray[i] % GridSize;
        }
    }
    MinValue;
    return MinValue;
}

int32 ACA_CityLayout::GetMinY(TArray<int32>& GridArray)
{
	if (GridArray.Num() == 0)
	{
		return 0;
	}
    return GridArray[0] / GridSize;
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
            //UE_LOG(LogTemp, Warning, TEXT("City Layout Simulation Iterations: %d"), Iterations);
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
			/*ISMBlocks->AddInstance(InstanceTransform);

			ISMBlocks->SetCustomDataValue(InstanceIndex, 0, 0);
            ISMBlocks->SetCustomDataValue(InstanceIndex, 1, 0);
            ISMBlocks->SetCustomDataValue(InstanceIndex, 2, 0);
            ISMBlocks->SetCustomDataValue(InstanceIndex, 3, 1);
            ISMBlocks->SetCustomDataValue(InstanceIndex, 4, 0.5);*/

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
	//UE_LOG(LogTemp, Warning, TEXT("Number of Junctions: %d"), Junctions.Num());
}

void ACA_CityLayout::FindLargestRectangle(TArray<int32>& ComponentIndices,int32& OutSizeX, int32& OutSizeY, int DistrictType)
{
    int32 minXSize;
    int32 minYSize;
    int32 maxXSize;
    int32 maxYSize;
    switch (DistrictType)
    {
    case 0:
        minXSize = 2;
        minYSize = 2;
        maxXSize = 10;
        maxYSize = 10;
        break;
    case 1:
        minXSize = minXSizeResidental;
        minYSize = minYSizeResidental;
        maxXSize = maxXSizeResidental;
        maxYSize = maxYSizeResidental;
        break;
    case 2:
        minXSize = minXSizeCommercial;
        minYSize = minYSizeCommercial;
        maxXSize = maxXSizeCommercial;
        maxYSize = maxYSizeCommercial;
        break;
    case 3:
        minXSize = minXSizeIndustrial;
        minYSize = minYSizeIndustrial;
        maxXSize = maxXSizeIndustrial;
        maxYSize = maxYSizeIndustrial;
        break;
    }
    TSet<FIntPoint> PointSet;

    // Konversi index 1D ke FIntPoint
    for (int32 Index : ComponentIndices)
    {
        int32 X = Index % GridSize;
        int32 Y = Index / GridSize;
        PointSet.Add(FIntPoint(X, Y));
    }

    int32 MaxArea = 0;
    FIntPoint BestTopLeft;
    int32 BestWidth = 0;
    int32 BestHeight = 0;

    // Coba semua titik sebagai top-left
    for (const FIntPoint& TL : PointSet)
    {
        // Maksimum lebarnya ditentukan oleh seberapa jauh ke kanan titik 1 masih valid
        int32 MaxW = 0;
        while (PointSet.Contains(FIntPoint(TL.X + MaxW, TL.Y))) ++MaxW;

        // Sekarang coba semua tinggi yang mungkin untuk lebar itu
        int32 MaxH = 0;
        bool Valid = true;
        while (Valid)
        {
            for (int32 dx = 0; dx < MaxW; ++dx)
            {
                if (!PointSet.Contains(FIntPoint(TL.X + dx, TL.Y + MaxH)))
                {
                    Valid = false;
                    break;
                }
            }
            if (Valid) ++MaxH;
        }

        int32 Area = MaxW * MaxH;
        if (Area > MaxArea)
        {
            MaxArea = Area;
            BestTopLeft = TL;
            BestWidth = MaxW;
            BestHeight = MaxH;
        }
    }
    ComponentIndices.Reset();
    // Bangun kembali array index 1D dari hasil terbaik
    if (BestHeight >= minXSize && BestWidth >= minYSize)
    {
        for (int32 y = 0; y < BestHeight; ++y)
        {
            for (int32 x = 0; x < BestWidth; ++x)
            {
                int32 GridIndex = (BestTopLeft.Y + y) * GridSize + (BestTopLeft.X + x);
                ComponentIndices.Add(GridIndex);
            }
        }
    }
    OutSizeX = BestWidth;
    OutSizeY = BestHeight;
}

int32 ACA_CityLayout::GetRoadType(int32 RoadIndex) const
{
    TArray<FIntPoint> Neighbors = GetMooreNeighbors(RoadIndex % GridSize, RoadIndex / GridSize);
    TArray<int32> RoadNeighborsIndex;
	for (int x = Neighbors.Num() - 1; x >= 0; x--)
	{
		int32 Index = GetIndex(Neighbors[x].X, Neighbors[x].Y);
        
        if (Grid[Index] <= ROAD) {
			Neighbors.RemoveAt(x); // Remove the road neighbor from the list
        }
	}
    for (int32 x = 0; x < Neighbors.Num(); x++) {
        int32 Index = GetIndex(Neighbors[x].X, Neighbors[x].Y);
        UE_LOG(LogTemp, Warning, TEXT("Grid[%d]: %d  x:%d   y:%d"), Index, Grid[Index], Neighbors[x].X, Neighbors[x].Y);
    }
	UE_LOG(LogTemp, Warning, TEXT("RoadIndex %d:  | x:%d y:%d |"), RoadIndex, RoadIndex % GridSize, RoadIndex / GridSize);
	UE_LOG(LogTemp, Warning, TEXT("============================="));
 //   for (FIntPoint Neighbor : Neighbors)
 //   {
 //       int32 NeighborIndex = GetIndex(Neighbor.X, Neighbor.Y);
	//	// Print Grid[NeighborIndex] for debugging
	//	UE_LOG(LogTemp, Warning, TEXT("Grid[%d]: %d  x:%d   y:%d"), NeighborIndex, Grid[NeighborIndex], Neighbor.X, Neighbor.Y);
 //       if (Grid[NeighborIndex] <= ROAD)
 //       {
 //           Neighbors.Remove(Neighbor); // Remove the road neighbor from the list
 //       }
 //       RoadNeighborsIndex.Add(NeighborIndex);
 //   }
	//// Print All Neighbors for that RoadIndex
 //   UE_LOG(LogTemp, Warning, TEXT("Neighbors for RoadIndex %d:  | x:%d y:%d |"), RoadIndex, RoadIndex % GridSize, RoadIndex / GridSize);
 //   for (const auto& Neighbor : Neighbors)
 //   {
	//UE_LOG(LogTemp, Warning, TEXT("Neighbor: (%d, %d) Index: %d"), Neighbor.X, Neighbor.Y, GetIndex(Neighbor.X, Neighbor.Y));
 //   }
    if (Neighbors.Num() == 0)
    {
        return 1; // No neighbors, return default road type
    }
    // Else if Has 1 neighbor and Contains Top Left Neighbor
	else if (Neighbors.Num() == 1 && Neighbors.Contains(FIntPoint(RoadIndex % GridSize - 1, RoadIndex / GridSize + 1)))
	{
		return 2; // Top Left Road
	}
	// Else if Has 1 neighbor and Contains Bot Left Neighbor
	else if (Neighbors.Num() == 1 && Neighbors.Contains(FIntPoint(RoadIndex % GridSize - 1, RoadIndex / GridSize - 1)))
    {
		return 3; // Bot Left Road
    }
	// Else if Has 1 neighbor and Contains Top Right Neighbor
	else if (Neighbors.Num() == 1 && Neighbors.Contains(FIntPoint(RoadIndex % GridSize + 1, RoadIndex / GridSize + 1)))
	{
		return 4; // Top Right Road
	}
	// Else if Has 1 neighbor and Contains Bot Right Neighbor
	else if (Neighbors.Num() == 1 && Neighbors.Contains(FIntPoint(RoadIndex % GridSize + 1, RoadIndex / GridSize - 1)))
	{
		return 5; // Bot Right Road
	}
	// Else if Has 2 neighbors and Contains Top Left and Top Right Neighbors
	else if (Neighbors.Num() == 2 && Neighbors.Contains(FIntPoint(RoadIndex % GridSize - 1, RoadIndex / GridSize + 1)) && Neighbors.Contains(FIntPoint(RoadIndex % GridSize + 1, RoadIndex / GridSize + 1)))
	{
		return 6; // Top Road
	}
	// Else if Has 2 neighbors and Contains Bot Left and Bot Right Neighbors
	else if (Neighbors.Num() == 2 && Neighbors.Contains(FIntPoint(RoadIndex % GridSize - 1, RoadIndex / GridSize - 1)) && Neighbors.Contains(FIntPoint(RoadIndex % GridSize + 1, RoadIndex / GridSize - 1)))
	{
		return 7; // Bot Road
	}
	// Else if Has 2 neighbors and Contains Top Right and Bot Right Neighbors
	else if (Neighbors.Num() == 2 && Neighbors.Contains(FIntPoint(RoadIndex % GridSize + 1, RoadIndex / GridSize + 1)) && Neighbors.Contains(FIntPoint(RoadIndex % GridSize + 1, RoadIndex / GridSize - 1)))
	{
		return 8; // Right Road
	}
	// Else if Has 2 neighbors and Contains Top Left and Bot Left Neighbors
	else if (Neighbors.Num() == 2 && Neighbors.Contains(FIntPoint(RoadIndex % GridSize - 1, RoadIndex / GridSize + 1)) && Neighbors.Contains(FIntPoint(RoadIndex % GridSize - 1, RoadIndex / GridSize - 1)))
	{
		return 9; // Left Road
	}
	// Else if Contains Left Neighbor and Not Contains Top Neighbor and Not Contains Bot Neighbor
	else if (Neighbors.Contains(FIntPoint(RoadIndex % GridSize - 1, RoadIndex / GridSize)) && !Neighbors.Contains(FIntPoint(RoadIndex % GridSize, RoadIndex / GridSize + 1)) && !Neighbors.Contains(FIntPoint(RoadIndex % GridSize, RoadIndex / GridSize - 1)))
	{
        UE_LOG(LogTemp, Warning, TEXT("x : %d, y : %d"), RoadIndex % GridSize - 1, RoadIndex / GridSize);
		return 10; // Left Road
	}
	// Else if Contains Top Neighbor and Not Contains Left Neighbor and Not Contains Right Neighbor
	else if (Neighbors.Contains(FIntPoint(RoadIndex % GridSize, RoadIndex / GridSize + 1)) && !Neighbors.Contains(FIntPoint(RoadIndex % GridSize - 1, RoadIndex / GridSize)) && !Neighbors.Contains(FIntPoint(RoadIndex % GridSize + 1, RoadIndex / GridSize)))
	{
        UE_LOG(LogTemp, Warning, TEXT("x : %d, y : %d"), RoadIndex % GridSize, RoadIndex / GridSize + 1);
		return 11; // Top Road
	}
	// Else if Contains Right Neighbor and Not Contains Top Neighbor and Not Contains Bot Neighbor
	else if (Neighbors.Contains(FIntPoint(RoadIndex % GridSize + 1, RoadIndex / GridSize)) && !Neighbors.Contains(FIntPoint(RoadIndex % GridSize, RoadIndex / GridSize + 1)) && !Neighbors.Contains(FIntPoint(RoadIndex % GridSize, RoadIndex / GridSize - 1)))
	{
		UE_LOG(LogTemp, Warning, TEXT("x : %d, y : %d"), RoadIndex % GridSize + 1, RoadIndex / GridSize);
		return 12; // Right Road
	}
	// Else if Contains Bot Neighbor and Not Contains Left Neighbor and Not Contains Right Neighbor
	else if (Neighbors.Contains(FIntPoint(RoadIndex % GridSize, RoadIndex / GridSize - 1)) && !Neighbors.Contains(FIntPoint(RoadIndex % GridSize - 1, RoadIndex / GridSize)) && !Neighbors.Contains(FIntPoint(RoadIndex % GridSize + 1, RoadIndex / GridSize)))
	{
        UE_LOG(LogTemp, Warning, TEXT("x : %d, y : %d"), RoadIndex % GridSize , RoadIndex / GridSize - 1);
		return 13; // Bot Road
	}
	// Else if Contains Left Neighbor and Contains Top Neighbor
	else if (Neighbors.Contains(FIntPoint(RoadIndex % GridSize - 1, RoadIndex / GridSize)) && Neighbors.Contains(FIntPoint(RoadIndex % GridSize, RoadIndex / GridSize + 1)))
    {
        return 14; // Left Top
    }
	// Else if Contains Top Neighbor and Contains Right Neighbor
	else if (Neighbors.Contains(FIntPoint(RoadIndex % GridSize, RoadIndex / GridSize + 1)) && Neighbors.Contains(FIntPoint(RoadIndex % GridSize + 1, RoadIndex / GridSize)))
	{
		return 15; // Top Right
	}
	// Else if Contains Left Neighbor and Contains Bot Neighbor
	else if (Neighbors.Contains(FIntPoint(RoadIndex % GridSize - 1, RoadIndex / GridSize)) && Neighbors.Contains(FIntPoint(RoadIndex % GridSize, RoadIndex / GridSize - 1)))
	{
		return 16; // Left Bot
	}
	// Else if Contains Right Neighbor and Contains Bot Neighbor
	else if (Neighbors.Contains(FIntPoint(RoadIndex % GridSize + 1, RoadIndex / GridSize)) && Neighbors.Contains(FIntPoint(RoadIndex % GridSize, RoadIndex / GridSize - 1)))
	{
		return 17; // Right Bot
	}
	// Else if Contains Left Neighbor and Contains Bot Neighbor and Contains Top Neighbor
	else if (Neighbors.Contains(FIntPoint(RoadIndex % GridSize - 1, RoadIndex / GridSize)) && Neighbors.Contains(FIntPoint(RoadIndex % GridSize, RoadIndex / GridSize - 1)) && Neighbors.Contains(FIntPoint(RoadIndex % GridSize, RoadIndex / GridSize + 1)))
	{
		return 18; // Left Bot Top
	}
	// Else if Contains Left Neighbor and Contains Top Neighbor and Contains Right Neighbor
	else if (Neighbors.Contains(FIntPoint(RoadIndex % GridSize - 1, RoadIndex / GridSize)) && Neighbors.Contains(FIntPoint(RoadIndex % GridSize, RoadIndex / GridSize + 1)) && Neighbors.Contains(FIntPoint(RoadIndex % GridSize + 1, RoadIndex / GridSize)))
	{
		return 19; // Left Top Right
	}
	// Else if Contains Right Neighbor and Contains Bot Neighbor and Contains Left Neighbor
	else if (Neighbors.Contains(FIntPoint(RoadIndex % GridSize + 1, RoadIndex / GridSize)) && Neighbors.Contains(FIntPoint(RoadIndex % GridSize, RoadIndex / GridSize - 1)) && Neighbors.Contains(FIntPoint(RoadIndex % GridSize - 1, RoadIndex / GridSize)))
	{
		return 20; // Right Bot Left
	}
	// Else if Contains Right Neighbor and Contains Top Neighbor and Contains Bot Neighbor
	else if (Neighbors.Contains(FIntPoint(RoadIndex % GridSize + 1, RoadIndex / GridSize)) && Neighbors.Contains(FIntPoint(RoadIndex % GridSize, RoadIndex / GridSize + 1)) && Neighbors.Contains(FIntPoint(RoadIndex % GridSize, RoadIndex / GridSize - 1)))
	{
		return 21; // Right Top Bot
	}


    return 0;
}

int32 ACA_CityLayout::GetMinDistanceToRoad(int32 X, int32 Y) const
{
	// Get the index of the cell
	int32 Index = GetIndex(X, Y);
	// If the cell is a road, return 0
	if (Grid[Index] == ROAD) {      
		return 0;
	}
	// Initialize the minimum distance to a large value
	int32 MinDistance = GridSize * 2; // Maximum possible distance in the grid
	int32 Radius = 1;
    while(true)
    {
        TArray<FIntPoint> Neighbors = GetMooreNeighborsWithinRadius(X, Y, Radius);
        // Loop through the neighbors
		for (const auto& Neighbor : Neighbors)
		{
			int32 NeighborIndex = GetIndex(Neighbor.X, Neighbor.Y);
			// If the neighbor is a road, calculate the distance
			if (Grid[NeighborIndex] == ROAD)
			{
				return Radius;
			}
            
		}
		Radius++;
		// If the radius exceeds the grid size, break the loop
        if (Radius > MinRoadAccessDistance) {
            break;
        }
    }
    return -1;
}



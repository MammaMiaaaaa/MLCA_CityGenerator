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

    // We want 3 floats per instance (R, G, B).
    InstancedGridMesh->NumCustomDataFloats = 3;


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
    const int32 MaxAttempts = GridSize * GridSize * 10;

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

void ACA_CityLayout::GetAllRoads(TArray<FRoadStruct> InRoadArray)
{
	TArray<int32> TempGrid = Grid;
	TArray<int32> RoadIndexArray;
	// For each cell in the grid, check if it's a road
	for (int32 y = 0; y < GridSize; ++y)
	{
		for (int32 x = 0; x < GridSize; ++x)
		{
			int32 Index = GetIndex(x, y);
			if (TempGrid[Index] == ROAD)
			{
				RecursiveWalkToRoads(TempGrid, Index, RoadIndexArray);
			}
		}
	}
}

void ACA_CityLayout::RecursiveWalkToRoads(TArray<int32>& GridArray, int32 Index, TArray<int32>& RoadIndexArray)
{
	// Set the current cell as visited
	GridArray[Index] = -3; // Mark as visited
	RoadIndexArray.Add(Index);

	// For each neighbor, check if it's a road
	TArray<FIntPoint> Neighbors = GetMooreNeighbors(Index % GridSize, Index / GridSize);
	for (const auto& N : Neighbors)
	{
		int32 NeighborIndex = GetIndex(N.X, N.Y);
		if (GridArray[NeighborIndex] == ROAD)
		{
            if (N.X == 0 || N.Y == 0 || N.X == GridSize - 1 || N.Y == GridSize - 1) {
				AddRoadsToArray(RoadIndexArray);
            }
            else {
				RecursiveWalkToRoads(GridArray, NeighborIndex, RoadIndexArray);
            }
		}
		else if (GridArray[NeighborIndex] == JUNCTION)
		{
			// If it's a junction, add it to the array
			RoadIndexArray.Add(NeighborIndex);
			AddRoadsToArray(RoadIndexArray);
		}
		
	}

}

void ACA_CityLayout::AddRoadsToArray(TArray<int32>& RoadIndexArray)
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
	RoadArray.Add(RoadStruct);
	// Clear the RoadIndexArray for the next road
	RoadIndexArray.Empty();


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
    TMap<int32, FLinearColor> DistrictColors;
    DistrictColors.Add(ROAD, FLinearColor::Black);
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
            InstanceTransform.SetLocation(FVector(x * CellSize, y * CellSize, 0.0f));

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
	}
	//print out the number of junctions
	UE_LOG(LogTemp, Warning, TEXT("Number of Junctions: %d"), Junctions.Num());
}


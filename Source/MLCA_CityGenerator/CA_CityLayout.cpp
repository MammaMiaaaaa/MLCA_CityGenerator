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

    TMap<int32, FLinearColor> DistrictColors;
    DistrictColors.Add(ACA_CityLayout::ROAD, FLinearColor::Black);

    for (int32 DistrictID = 1; DistrictID <= NumDistricts; ++DistrictID)
    {
        // Pick random color. Or generate systematically.
        FLinearColor RandomColor = FLinearColor::MakeRandomColor();
        DistrictColors.Add(DistrictID, RandomColor);
    }
}

// Called when the game starts or when spawned
void ACA_CityLayout::BeginPlay()
{
	Super::BeginPlay();

	/*Initialize();
	Simulate();
	VisualizeGrid();*/

	// Print Start
	//UE_LOG(LogTemp, Warning, TEXT("City Layout Simulation Started"));

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

            TArray<FIntPoint> Neighbors = GetMooreNeighbors(x, y);
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

void ACA_CityLayout::Simulate()
{
    TArray<int32> NewGrid;
	int32 SameCount = 0;
    while (Grid.Contains(EMPTY))
    {
        GrowDistricts(NewGrid);
        if (NewGrid == Grid)
			SameCount++;
		if (SameCount > 3)
            break;

        Grid = NewGrid;
        Iterations++;
    }

    AddRoads(Grid);
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
            const float CellSize = 100.0f;
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


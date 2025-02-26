// Fill out your copyright notice in the Description page of Project Settings.


#include "CA_Grid_2D.h"
#include "Components/InstancedStaticMeshComponent.h"

// Sets default values
ACA_Grid_2D::ACA_Grid_2D()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	InstancedStaticMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Instanced Static Mesh"));
	InstancedStaticMeshComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ACA_Grid_2D::BeginPlay()
{
	Super::BeginPlay();

	Random.Initialize(Seed);
	Map.Init(1, MapSize.X * MapSize.Y);

	for (int y = 0; y < MapSize.Y; y++)
	{
		for (int x = 0; x < MapSize.X; x++)
		{
			Map[y * MapSize.X + x] = static_cast<int>(Random.FRand() < AliveChance);
		}
	}

	RenderInstancedCells();
	
}

// Called every frame
void ACA_Grid_2D::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACA_Grid_2D::StepIteration()
{
	Step();
	RenderInstancedCells();
}

void ACA_Grid_2D::Finalize()
{
	for (int i = 0; i < 15; i++)
	{
		Step();
	}

	RenderInstancedCells();
}

void ACA_Grid_2D::RenderInstancedCells()
{
	InstancedStaticMeshComponent->ClearInstances();
	for (int y = 0; y < MapSize.Y; y++)
	{
		for (int x = 0; x < MapSize.X; x++)
		{
			const int Index = y * MapSize.X + x;
			if (Map[Index] == 0)
			{
				continue;
			}

			FTransform Transform(FVector(x * 100, y * 100, 0));
			InstancedStaticMeshComponent->AddInstance(Transform);
		}
	}
}
void ACA_Grid_2D::RenderDefault()
{
	RenderInstancedCells();
}

void ACA_Grid_2D::RenderInverted()
{
	InstancedStaticMeshComponent->ClearInstances();
	for (int y = 0; y < MapSize.Y; y++)
	{
		for (int x = 0; x < MapSize.X; x++)
		{
			const int Index = y * MapSize.X + x;
			if (Map[Index] == 1)
			{
				continue;
			}

			FTransform Transform(FVector(x * 100, y * 100, 0));
			InstancedStaticMeshComponent->AddInstance(Transform);
		}
	}
}

void ACA_Grid_2D::GroupingCellularAutomataRules()
{
	TArray<int> NewMap = Map;
	for (int y = 0; y < MapSize.Y; y++)
	{
		for (int x = 0; x < MapSize.X; x++)
		{
			const int Index = y * MapSize.X + x;
			const int Count = CountCellWalls(FVector2D(x, y));
			if (Map[Index] == 0 && Count > 3)
			{
				NewMap[Index] = 1;
			}
			if (Map[Index] == 1 && Count < 5)
			{
				NewMap[Index] = 0;
			}
		}
	}

	Map = NewMap;
}

void ACA_Grid_2D::SmoothingCellularAutomataRules()
{
	TArray<int> NewMap = Map;
	for (int y = 0; y < MapSize.Y; y++)
	{
		for (int x = 0; x < MapSize.X; x++)
		{
			const int Index = y * MapSize.X + x;
			const int Count = CountCellWalls(FVector2D(x, y));
			if (Map[Index] == 0 && Count > 4)
			{
				NewMap[Index] = 1;
			}
			if (Map[Index] == 1 && Count < 4)
			{
				NewMap[Index] = 0;
			}
		}
	}

	Map = NewMap;
}

int ACA_Grid_2D::CountCellWalls(FVector2D Coordinate)
{
	int WallCount = 0;
	TArray Neighbours = {
		FVector2D(-1, 1),
		FVector2D(0, 1),
		FVector2D(1, 1),
		FVector2D(-1, 0),
		FVector2D(1, 0),
		FVector2D(-1, -1),
		FVector2D(0, -1),
		FVector2D(1, -1),
	};

	for (auto Neighbour : Neighbours)
	{
		const auto Cell = Coordinate + Neighbour;
		if (!IsValidCoordinate(Cell))
		{
			WallCount += 1;
		}
		else
		{
			const int Index = Cell.Y * MapSize.X + Cell.X;
			if (Map[Index] == 1)
			{
				WallCount += 1;
			}
		}
	}

	return WallCount;
}

bool ACA_Grid_2D::IsValidCoordinate(FVector2D Coordinate) const
{
	return Coordinate.X >= 0 && Coordinate.X < MapSize.X
		&& Coordinate.Y >= 0 && Coordinate.Y < MapSize.Y;
}

void ACA_Grid_2D::Step()
{
	TArray<int> NewMap = Map;
	for (int y = 0; y < MapSize.Y; y++)
	{
		for (int x = 0; x < MapSize.X; x++)
		{
			const int Index = y * MapSize.X + x;
			const int Count = CountCellWalls(FVector2D(x, y));
			if (Map[Index] == 1)
			{
				NewMap[Index] = !(Count < DeathLimit);
			}
			else
			{
				NewMap[Index] = Count > BirthLimit;
			}
		}
	}

	Map = NewMap;
}






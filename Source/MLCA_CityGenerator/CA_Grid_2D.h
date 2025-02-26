// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CA_Grid_2D.generated.h"

UCLASS()
class MLCA_CITYGENERATOR_API ACA_Grid_2D : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UInstancedStaticMeshComponent* InstancedStaticMeshComponent;

	UPROPERTY()
	TArray<int> Map;

	UPROPERTY(EditAnywhere)
	FVector2D MapSize = { 64, 64 };

	UPROPERTY(EditAnywhere)
	int Seed = 1337;

	UPROPERTY(EditAnywhere)
	int DeathLimit = 3;

	UPROPERTY(EditAnywhere)
	int BirthLimit = 4;

	UPROPERTY(EditAnywhere)
	float AliveChance = 0.45f;

	FRandomStream Random;
	
public:	
	// Sets default values for this actor's properties
	ACA_Grid_2D();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	const TArray<int>& GetMap() const { return Map; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	const FVector2D& GetMapSize() const { return MapSize; }

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Grid")
	void StepIteration();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Grid")
	void Finalize();

	UFUNCTION(BlueprintCallable)
	void RenderInstancedCells();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Grid")
	void RenderDefault();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Grid")
	void RenderInverted();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Grid")
	void GroupingCellularAutomataRules();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Grid")
	void SmoothingCellularAutomataRules();

protected:
	UFUNCTION()
	int CountCellWalls(FVector2D Coordinate);

	UFUNCTION()
	bool IsValidCoordinate(FVector2D Coordinate) const;

	UFUNCTION()
	void Step();
};

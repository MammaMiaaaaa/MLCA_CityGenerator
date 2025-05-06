// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DistrictTypeEnum.h"
#include "DistrictStruct.generated.h"

USTRUCT(BlueprintType)
struct FDistrictStruct
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "District")
	int32 DistrictID = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "District")
	int32 DistrictType = 0; // 0 = Empty, 1 = Residential, 2 = Commercial, 3 = Industrial
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "District")
	TArray<FVector> DistrictCellLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "District")
	TArray<int32> DistrictCellIndex;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "District")
	TArray<EDistrictTypeEnum> AvailableDistrictType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "District")
	float WaterAvailValue = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "District")
	float ElectricityAvailValue = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "District")
	float PopulationSatisfactionValue = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "District")
	float PolutionValue = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "District")
	float PopulationDensityValue = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "District")
	float RoadAccessibilityValue = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "District")
	float SecurityValue = 0.0f;
	
};
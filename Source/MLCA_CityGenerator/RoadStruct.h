// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h" // Ensure this is included for UObject-related macros
#include "RoadStruct.generated.h"

USTRUCT(BlueprintType)
struct FRoadStruct
{
GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road")
TArray<FVector> RoadPointsLocation;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road")
TArray<int32> RoadPointsIndex;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road")
TArray<int32> JunctionIndex;
};
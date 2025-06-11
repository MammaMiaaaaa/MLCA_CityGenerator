#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GridToJsonConverter.generated.h"

/**
 * A utility class for converting grid data to JSON format.
 */
UCLASS()
class MLCA_CITYGENERATOR_API UGridToJsonConverter : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Converts a flat TArray of integers into a 2D JSON grid format.
     *
     * @param FlatGridData A TArray of integers representing the grid in a flat 1D format. Expects 625 elements for a 25x25 grid.
     * @param GridSize The dimension of the square grid (e.g., 25 for a 25x25 grid).
     * @return A FString containing the formatted JSON. Returns an empty string if the input data is invalid.
     */
    UFUNCTION(BlueprintCallable, Category = "JSON Conversion")
    static FString ConvertIntArrayTo2DJsonGrid(const TArray<int32>& FlatGridData, int32 GridSize);
};
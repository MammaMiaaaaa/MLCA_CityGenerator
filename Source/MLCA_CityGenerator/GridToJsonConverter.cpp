#include "GridToJsonConverter.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

FString UGridToJsonConverter::ConvertIntArrayTo2DJsonGrid(const TArray<int32>& FlatGridData, int32 GridSize)
{
    // --- Input Validation ---
    // Ensure the grid size is positive and the array has the expected number of elements.
    if (GridSize <= 0 || FlatGridData.Num() != (GridSize * GridSize))
    {
        UE_LOG(LogTemp, Error, TEXT("ConvertIntArrayTo2DJsonGrid: Invalid input. GridSize must be > 0 and FlatGridData.Num() must equal GridSize*GridSize."));
        return TEXT("");
    }

    // --- JSON Construction ---

    // 1. Create the root JSON object that will hold our grid.
    // This is the main container, equivalent to the outermost {}
    TSharedPtr<FJsonObject> RootJsonObject = MakeShareable(new FJsonObject());

    // 2. Create the main JSON array that will hold all the rows.
    // This will become the value for the "grid" key.
    TArray<TSharedPtr<FJsonValue>> RowsArray;

    // 3. Iterate through the flat data to build the 2D structure.
    for (int32 RowIndex = 0; RowIndex < GridSize; ++RowIndex)
    {
        // For each row, create a new array to hold its column values.
        TArray<TSharedPtr<FJsonValue>> ColumnsArray;

        for (int32 ColIndex = 0; ColIndex < GridSize; ++ColIndex)
        {
            // Calculate the index in the flat 1D source array.
            const int32 FlatIndex = (RowIndex * GridSize) + ColIndex;

            // Get the integer value.
            const int32 Value = FlatGridData[FlatIndex];

            // Add the integer to the columns array for the current row.
            // FJsonValueNumber is used to represent a numeric JSON value.
            ColumnsArray.Add(MakeShareable(new FJsonValueNumber(Value)));
        }

        // Once a row is complete, wrap the ColumnsArray in a FJsonValueArray
        // and add it to the main RowsArray.
        RowsArray.Add(MakeShareable(new FJsonValueArray(ColumnsArray)));
    }

    // 4. Assign the completed RowsArray to the "grid" field in our root object.
    RootJsonObject->SetArrayField(TEXT("grid"), RowsArray);

    // --- Serialization to String ---

    // 5. Create a string to hold the final output.
    FString OutputJsonString;

    // 6. Create a writer and serialize the root FJsonObject into the string.
    // The TJsonWriterFactory creates a writer that outputs a condensed, non-pretty string.
    TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutputJsonString);
    FJsonSerializer::Serialize(RootJsonObject.ToSharedRef(), JsonWriter);

    return OutputJsonString;
}

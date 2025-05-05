#pragma once

#include "CoreMinimal.h"
#include "DistrictTypeEnum.generated.h"

UENUM(BlueprintType)
enum class EDistrictTypeEnum : uint8
{
	Empty UMETA(DisplayName = "Empty"),
	Residential UMETA(DisplayName = "Residential"),
	Commercial UMETA(DisplayName = "Commercial"),
	Industrial UMETA(DisplayName = "Industrial")
};
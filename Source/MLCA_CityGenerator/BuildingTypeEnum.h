#pragma once
UENUM(BlueprintType)
enum class EBuildingTypeEnum: uint8
{
	WaterTower UMETA(DisplayName = "WaterTower"),
	ElectricityTower UMETA(DisplayName = "ElectricityTower"),
	School UMETA(DisplayName = "School"),
	Park UMETA(DisplayName = "Park"),
	PoliceStation UMETA(DisplayName = "PoliceStation"),
	
};
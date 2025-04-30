UENUM(BlueprintType)
enum class ELayerEnum : uint8
{
    Road UMETA(DisplayName = "RoadLayer"),
    District UMETA(DisplayName = "DistrictLayer"),
    Water UMETA(DisplayName = "WaterLayer"),
    Electricity UMETA(DisplayName = "ElectricityLayer"),
	Satisfaction UMETA(DisplayName = "SatisfactionLayer"),
	Polution UMETA(DisplayName = "PolutionLayer"),
	Density UMETA(DisplayName = "DensityLayer"),
	Accessibility UMETA(DisplayName = "AccessibilityLayer"),
	Security UMETA(DisplayName = "SecurityLayer"),
	Grid UMETA(DisplayName = "GridLayer"),
	None UMETA(DisplayName = "None")
};
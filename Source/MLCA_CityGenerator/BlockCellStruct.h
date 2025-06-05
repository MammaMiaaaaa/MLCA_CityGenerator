// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlockCellStruct.generated.h"

USTRUCT(BlueprintType)
struct FBlockCellStruct
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block")
	int32 BlockID = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block")
	TArray<int32> BlockArray;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block")
	int32 SizeX = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block")
	int32 SizeY = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block")
	int32 BuildingDirection = 1; // 1 = Left , 2 = Top, 3 = Right, 4 = Bottom
	

};
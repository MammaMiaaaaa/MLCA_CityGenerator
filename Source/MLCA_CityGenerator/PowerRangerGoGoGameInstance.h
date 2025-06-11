// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "IWebSocket.h"
#include "PowerRangerGoGoGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class MLCA_CITYGENERATOR_API UPowerRangerGoGoGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;
	
	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void SendGridMessage(const TArray<int32>& GridData);
	

	TSharedPtr<IWebSocket> WebSocket;

	
};

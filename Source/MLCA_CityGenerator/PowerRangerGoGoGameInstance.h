// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "IWebSocket.h"
#include "PowerRangerGoGoGameInstance.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketMessageReceived, const FString&, Message);

UCLASS()
class MLCA_CITYGENERATOR_API UPowerRangerGoGoGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;
	
	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void SendGridMessage(const TArray<int32>& GridData);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString WebsocketResponds = "None";

	UPROPERTY(BlueprintAssignable)
	FOnWebSocketMessageReceived OnWebSocketMessageReceived;
	

	TSharedPtr<IWebSocket> WebSocket;

	
};

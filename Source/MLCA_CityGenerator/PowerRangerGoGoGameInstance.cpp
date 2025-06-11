// Fill out your copyright notice in the Description page of Project Settings.


#include "PowerRangerGoGoGameInstance.h"
#include "GridToJsonConverter.h"
#include "WebSocketsModule.h"

void UPowerRangerGoGoGameInstance::Init()
{
	Super::Init();
	if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
	{
		FModuleManager::Get().LoadModule("WebSockets");
	}

	// Create Websocket on game start
	WebSocket = FWebSocketsModule::Get().CreateWebSocket(TEXT("ws://localhost:8765")); // Replace with your WebSocket URL

	// Add event handlers for WebSocket events
	WebSocket->OnConnected().AddLambda([]() {
		UE_LOG(LogTemp, Log, TEXT("WebSocket connected!"));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "WebSocket connected successfully!");
		});
	WebSocket->OnConnectionError().AddLambda([](const FString& Error) {
		UE_LOG(LogTemp, Error, TEXT("WebSocket connection error: %s"), *Error);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "WebSocket connection error: " + Error);
		});
	WebSocket->OnClosed().AddLambda([](int32 StatusCode, const FString& Reason, bool bWasClean)
		{
			// Log the closure of the WebSocket connection
			UE_LOG(LogTemp, Log, TEXT("WebSocket closed: %d, %s, Clean: %s"), StatusCode, *Reason, bWasClean ? TEXT("true") : TEXT("false"));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, bWasClean ? FColor::Green : FColor::Red, "Connection closed " + Reason);
			
		});

	// Add event handler for message received
	WebSocket->OnMessage().AddLambda([](const FString& Message)
		{
			UE_LOG(LogTemp, Log, TEXT("WebSocket message received: %s"), *Message);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, "Message received: " + Message);
		});

	
	// On Message sent
	WebSocket->OnMessageSent().AddLambda([](const FString& Message)
		{
			UE_LOG(LogTemp, Log, TEXT("WebSocket message sent: %s"), *Message);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "Message sent: " + Message);
		});

	// Connect to Websocket
	WebSocket->Connect();

}

void UPowerRangerGoGoGameInstance::Shutdown()
{
	// Check if websocket is connected before closing
	if (WebSocket->IsConnected())
	{
		WebSocket->Close();
	}
	
	Super::Shutdown();
}

void UPowerRangerGoGoGameInstance::SendGridMessage(const TArray<int32>& GridData)
{
	if (!WebSocket.IsValid() || !WebSocket->IsConnected())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot send message, WebSocket is not connected."));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "WebSocket is not connected!");
		return;
	}

	// Use your static function to convert the array to a JSON string.
	// We assume a 25x25 grid here as per your original request.
	const int32 GridSize = 25;
	FString JsonString = UGridToJsonConverter::ConvertIntArrayTo2DJsonGrid(GridData, GridSize);

	// Ensure the JSON string is not empty before sending
	if (!JsonString.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("Sending Grid JSON to server..."));
		WebSocket->Send(JsonString);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to generate grid JSON string. Message not sent."));
	}
}



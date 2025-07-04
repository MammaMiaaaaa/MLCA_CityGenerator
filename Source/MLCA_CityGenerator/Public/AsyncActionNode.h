// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncActionNode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTaskOutput);
/**
 * 
 */
UCLASS()
class MLCA_CITYGENERATOR_API UAsyncActionNode : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintAssignable)
	FTaskOutput MultiThreadedWork;

	UPROPERTY(BlueprintAssignable)
	FTaskOutput FinishedWork;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UAsyncActionNode* MultiThreadedNode();

	virtual void Activate() override;
};

//==========================================

class BP_NonAbandonedTask : public FNonAbandonableTask
{
public:

	BP_NonAbandonedTask(UAsyncActionNode* BP_TaskInstance);

	~BP_NonAbandonedTask();

	//UE 5 need this
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(BP_NonAbandonedTask, STATGROUP_ThreadPoolAsyncTasks);
	}

	UAsyncActionNode* CallingObject = nullptr;

	void DoWork();

};
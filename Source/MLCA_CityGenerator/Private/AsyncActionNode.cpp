// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncActionNode.h"

UAsyncActionNode* UAsyncActionNode::MultiThreadedNode()
{
	UAsyncActionNode* Node = NewObject<UAsyncActionNode>();
	return Node;
}

void UAsyncActionNode::Activate()
{
	(new FAutoDeleteAsyncTask<BP_NonAbandonedTask>(this))->StartBackgroundTask();
}

//============================================

BP_NonAbandonedTask::BP_NonAbandonedTask(UAsyncActionNode* BP_TaskInstance)
{
	CallingObject = BP_TaskInstance;
}

BP_NonAbandonedTask::~BP_NonAbandonedTask()
{
	CallingObject->FinishedWork.Broadcast();
	CallingObject->SetReadyToDestroy();
}

void BP_NonAbandonedTask::DoWork()
{
	CallingObject->MultiThreadedWork.Broadcast();
}

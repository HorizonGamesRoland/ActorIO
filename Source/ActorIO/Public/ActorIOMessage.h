// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "ActorIOMessage.generated.h"

class UActorIOAction;

USTRUCT()
struct FActorIOMessage
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UActorIOAction> OwningAction;

	UPROPERTY()
	TObjectPtr<UObject> TargetObject;

	FString Command;

	FTimerHandle TimerHandle_ExecutionDelay;

	FActorIOMessage() :
		OwningAction(nullptr),
		TargetObject(nullptr),
		Command(FString()),
		TimerHandle_ExecutionDelay(FTimerHandle())
	{}

	bool Invoke() const;
};

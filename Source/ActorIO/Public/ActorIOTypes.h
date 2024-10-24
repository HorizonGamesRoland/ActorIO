// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "Delegates/Delegate.h"
#include "ActorIOTypes.generated.h"

USTRUCT()
struct ACTORIO_API FActorIOEvent
{
	GENERATED_BODY()

	FName EventName;

	FMulticastScriptDelegate* EventDelegate;

	FActorIOEvent() :
		EventName(FName()),
		EventDelegate(nullptr)
	{}

	FActorIOEvent(FName InEventName, FMulticastScriptDelegate* InDelegate) :
		EventName(InEventName),
		EventDelegate(InDelegate)
	{}

	bool operator==(const FName InEventName) const
	{
		return EventName == InEventName;
	}
};

USTRUCT()
struct ACTORIO_API FActorIOFunction
{
	GENERATED_BODY()

	FName FunctionName;

	FName FunctionToExec;

	FActorIOFunction() :
		FunctionName(FName()),
		FunctionToExec(FName())
	{}

	FActorIOFunction(FName InFunctionName, FName InFunctionToExec) :
		FunctionName(InFunctionName),
		FunctionToExec(InFunctionToExec)
	{}

	bool operator==(const FName InFunctionName) const
	{
		return FunctionName == InFunctionName;
	}
};

USTRUCT(BlueprintType)
struct ACTORIO_API FActorIOAction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FName SourceEvent;

	UPROPERTY(EditAnywhere)
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(EditAnywhere)
	FName TargetFunction;

	FActorIOAction() :
		SourceEvent(FName()),
		TargetActor(nullptr),
		TargetFunction(FName())
	{}

	bool IsValid() const
	{
		return !SourceEvent.IsNone() && TargetActor && !TargetFunction.IsNone();
	}
};

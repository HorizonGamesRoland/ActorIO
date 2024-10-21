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

	FGuid ActionId;

	FName SourceEvent;

	UPROPERTY()
	TObjectPtr<AActor> TargetActor;

	FName TargetFunction;

	FDelegateHandle ActionDelegateHandle;

	FActorIOAction() :
		ActionId(FGuid::NewGuid()),
		SourceEvent(FName()),
		TargetActor(nullptr),
		TargetFunction(FName()),
		ActionDelegateHandle(FDelegateHandle())
	{}

	bool operator==(const FGuid& InBindId) const
	{
		return ActionId == InBindId;
	}

	bool operator==(const FActorIOAction& Other) const
	{
		return ActionId == Other.ActionId;
	}

	bool IsValid() const
	{
		return ActionId.IsValid() && !SourceEvent.IsNone() && TargetActor && !TargetFunction.IsNone();
	}
};

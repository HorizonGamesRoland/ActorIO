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




UENUM()
enum class EActorIONativeEvents : uint32
{
	ActorBeginOverlap,

	ActorEndOverlap
};

static FName ToName(EActorIONativeEvents InEnumValue)
{
	return UEnum::GetValueAsName(InEnumValue);
}





USTRUCT()
struct ACTORIO_API FActorIOFunction
{
	GENERATED_BODY()

	FName FunctionName;

	FString FunctionToExec;

	FActorIOFunction() :
		FunctionName(FName()),
		FunctionToExec(FString())
	{}

	FActorIOFunction(FName InFunctionName, FString InFunctionToExec) :
		FunctionName(InFunctionName),
		FunctionToExec(InFunctionToExec)
	{}

	bool operator==(const FName InFunctionName) const
	{
		return FunctionName == InFunctionName;
	}
};



UENUM()
enum class EActorIONativeFunctions : uint32
{
	SetActorHiddenInGame
};

static FName ToName(EActorIONativeFunctions InEnumValue)
{
	return UEnum::GetValueAsName(InEnumValue);
}




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

	UPROPERTY(EditAnywhere)
	FString FunctionArguments;

	FActorIOAction() :
		SourceEvent(FName()),
		TargetActor(nullptr),
		TargetFunction(FName()),
		FunctionArguments(FString())
	{}

	bool IsValid() const
	{
		return !SourceEvent.IsNone() && TargetActor && !TargetFunction.IsNone();
	}
};

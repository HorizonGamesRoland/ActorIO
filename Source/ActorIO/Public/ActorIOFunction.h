// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "ActorIOFunction.generated.h"

USTRUCT()
struct ACTORIO_API FActorIOFunction
{
	GENERATED_BODY()

	FName FunctionId;

	FText DisplayName;

	FText TooltipText;

	FString FunctionToExec;

	FActorIOFunction() :
		FunctionId(NAME_None),
		DisplayName(FText::GetEmpty()),
		TooltipText(FText::GetEmpty()),
		FunctionToExec(FString())
	{}

	FActorIOFunction& SetId(FName InFunctionId)
	{
		FunctionId = InFunctionId;
		return *this;
	}

	FActorIOFunction& SetDisplayName(const FText& InDisplayName)
	{
		DisplayName = InDisplayName;
		return *this;
	}

	FActorIOFunction& SetTooltipText(const FText& InTooltipText)
	{
		TooltipText = InTooltipText;
		return *this;
	}

	FActorIOFunction& SetFunction(const FString& InFunctionName)
	{
		FunctionToExec = InFunctionName;
		return *this;
	}

	bool operator==(const FName InFunctionName) const
	{
		return FunctionId == InFunctionName;
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

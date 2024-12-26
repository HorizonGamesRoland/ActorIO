// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "ActorIOFunction.generated.h"

USTRUCT(BlueprintType)
struct ACTORIO_API FActorIOFunction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Function")
	FName FunctionId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Function")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Function")
	FText TooltipText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Function")
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

	bool operator==(const FName InFunctionId) const
	{
		return FunctionId == InFunctionId;
	}
};

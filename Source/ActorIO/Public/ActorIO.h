// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "UObject/ScriptDelegates.h"
#include "UObject/ScriptDelegateFwd.h"
#include "UObject/SparseDelegate.h"
#include "ActorIO.generated.h"

class UActorIOAction;
class UObject;

ACTORIO_API DECLARE_LOG_CATEGORY_EXTERN(LogActorIO, Log, All);

USTRUCT(BlueprintType)
struct ACTORIO_API FActorIOEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FName EventId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FText TooltipText;

	UPROPERTY()
	TObjectPtr<UObject> DelegateOwner;

	FMulticastScriptDelegate* MulticastDelegatePtr;

	FName SparseDelegateName;

	FName BlueprintDelegateName;

	FScriptDelegate EventProcessor;

	FActorIOEvent() :
		EventId(NAME_None),
		DisplayName(FText::GetEmpty()),
		TooltipText(FText::GetEmpty()),
		DelegateOwner(nullptr),
		MulticastDelegatePtr(nullptr),
		SparseDelegateName(NAME_None),
		BlueprintDelegateName(NAME_None),
		EventProcessor(FScriptDelegate())
	{}

	FActorIOEvent& SetId(FName InEventId)
	{
		EventId = InEventId;
		return *this;
	}

	FActorIOEvent& SetDisplayName(const FText& InDisplayName)
	{
		DisplayName = InDisplayName;
		return *this;
	}

	FActorIOEvent& SetTooltipText(const FText& InTooltipText)
	{
		TooltipText = InTooltipText;
		return *this;
	}

	FActorIOEvent& SetMulticastDelegate(UObject* InDelegateOwner, FMulticastScriptDelegate* InMulticastDelegate)
	{
		DelegateOwner = InDelegateOwner;
		MulticastDelegatePtr = InMulticastDelegate;
		return *this;
	}

	FActorIOEvent& SetSparseDelegate(UObject* InDelegateOwner, FName InSparseDelegateName)
	{
		DelegateOwner = InDelegateOwner;
		SparseDelegateName = InSparseDelegateName;
		return *this;
	}

	FActorIOEvent& SetBlueprintDelegate(UObject* InDelegateOwner, FName InBlueprintDelegateName)
	{
		DelegateOwner = InDelegateOwner;
		BlueprintDelegateName = InBlueprintDelegateName;
		return *this;
	}

	FActorIOEvent& SetEventProcessor(UObject* InFunctionOwner, FName InFunctionName)
	{
		EventProcessor.BindUFunction(InFunctionOwner, InFunctionName);
		return *this;
	}

	bool operator==(const FName InEventName) const
	{
		return EventId == InEventName;
	}

	bool operator!=(const FName InEventName) const
	{
		return EventId != InEventName;
	}
};

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

	bool operator!=(const FName InFunctionId) const
	{
		return FunctionId != InFunctionId;
	}
};

USTRUCT()
struct ACTORIO_API FActionExecutionContext
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UActorIOAction> ActionPtr;

	void* ScriptParams;

	TMap<FString, FString> NamedArguments;

	FActionExecutionContext() :
		ActionPtr(nullptr),
		ScriptParams(nullptr),
		NamedArguments(TMap<FString, FString>())
	{}

	static FActionExecutionContext& Get(UObject* WorldContextObject);

	void EnterContext(UActorIOAction* InAction, void* InScriptParams);

	void ExitContext();

	bool HasContext() const;

	void AddNamedArgument(const FString& InName, const FString& InValue);

	void RemoveNamedArgument(const FString& InName);
};

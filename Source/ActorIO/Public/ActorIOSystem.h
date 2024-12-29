// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "ActorIOEvent.h"
#include "ActorIOFunction.h"
#include "Subsystems/WorldSubsystem.h"
#include "ActorIOSystem.generated.h"

class UActorIOAction;

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

	void LeaveContext();

	bool HasContext() const;

	void AddNamedArgument(const FString& InName, const FString& InValue)
	{
		FString Arg = NamedArguments.FindOrAdd(InName);
		Arg = InValue;
	}

	void RemoveNamedArgument(const FString& InName)
	{
		NamedArguments.Remove(InName);
	}
};

UCLASS()
class ACTORIO_API UActorIOSystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	UActorIOSystem();

private:

	UPROPERTY(Transient)
	FActionExecutionContext ActionExecContext;

public:

	FActionExecutionContext& GetExecutionContext() { return ActionExecContext; }

public:

	static TArray<FActorIOEvent> GetEventsForObject(AActor* InObject);

	static TArray<FActorIOFunction> GetFunctionsForObject(AActor* InObject);

	static TArray<TWeakObjectPtr<UActorIOAction>> GetInputActionsForObject(const AActor* InObject);

	static int32 GetNumInputActionsForObject(const AActor* InObject);

protected:

	UFUNCTION(BlueprintCallable, Category = "Actor IO", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "DisplayName,TooltipText", AdvancedDisplay = "DisplayName,TooltipText"))
	static void RegisterIOEvent(UObject* WorldContextObject, UPARAM(Ref) TArray<FActorIOEvent>& RegisterTo, FName EventId, FName EventDispatcherName, const FText& DisplayName, const FText & TooltipText);

	UFUNCTION(BlueprintCallable, Category = "Actor IO", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "DisplayName,TooltipText", AdvancedDisplay = "DisplayName,TooltipText"))
	static void RegisterIOFunction(UObject* WorldContextObject, UPARAM(Ref) TArray<FActorIOFunction>& RegisterTo, FName FunctionId, FString FunctionToExec, const FText& DisplayName, const FText& TooltipText);

private:

	static void GetNativeEventsForObject(AActor* InObject, TArray<FActorIOEvent>& RegisteredEvents);

	static void GetNativeFunctionsForObject(AActor* InObject, TArray<FActorIOFunction>& RegisteredFunctions);
};

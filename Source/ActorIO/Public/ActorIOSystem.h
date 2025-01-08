// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "Subsystems/WorldSubsystem.h"
#include "ActorIOSystem.generated.h"

class UActorIOAction;

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

	static FActorIOEventList GetEventsForObject(AActor* InObject); // #TODO: make object const?

	static FActorIOFunctionList GetFunctionsForObject(AActor* InObject);

	static TArray<TWeakObjectPtr<UActorIOAction>> GetInputActionsForObject(const AActor* InObject);

	static int32 GetNumInputActionsForObject(const AActor* InObject);

protected:

	UFUNCTION(BlueprintCallable, Category = "Actor IO", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "DisplayName,TooltipText", AdvancedDisplay = "DisplayName,TooltipText"))
	static void RegisterIOEvent(UObject* WorldContextObject, UPARAM(Ref) FActorIOEventList& RegisterTo, FName EventId, FName EventDispatcherName, const FText& DisplayName, const FText& TooltipText);

	UFUNCTION(BlueprintCallable, Category = "Actor IO", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "DisplayName,TooltipText", AdvancedDisplay = "DisplayName,TooltipText"))
	static void RegisterIOFunction(UObject* WorldContextObject, UPARAM(Ref) FActorIOFunctionList& RegisterTo, FName FunctionId, FString FunctionToExec, const FText& DisplayName, const FText& TooltipText);

private:

	static void GetNativeEventsForObject(AActor* InObject, FActorIOEventList& RegisteredEvents);

	static void GetNativeFunctionsForObject(AActor* InObject, FActorIOFunctionList& RegisteredFunctions);
};

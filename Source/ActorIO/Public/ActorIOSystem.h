// Copyright 2025 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "Subsystems/WorldSubsystem.h"
#include "ActorIOSystem.generated.h"

class UActorIOAction;

/**
 * World subsystem of the I/O system.
 * Implements global functions, and stores the current action execution context.
 */
UCLASS(DisplayName = "Actor I/O System")
class ACTORIO_API UActorIOSystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UActorIOSystem();

public:

	/**
	 * The current I/O action execution context.
	 * Only valid between an action receiving the ProcessEvent call and sending the command to the target actor.
	 */
	UPROPERTY(Transient)
	FActionExecutionContext ActionExecContext;

public:

	/**
	 * Get the current I/O action execution context.
	 * In general you should use FActionExecutionContext::Get() instead.
	 */
	FActionExecutionContext& GetExecutionContext() { return ActionExecContext; }

public:

	/** @return List of registered I/O events of the given actor. */
	static FActorIOEventList GetEventsForObject(AActor* InObject);

	/** @return List of registered I/O functions of the given actor. */
	static FActorIOFunctionList GetFunctionsForObject(AActor* InObject);

	/** @return List of I/O actions currently loaded in the world that are targeting the given actor. */
	static TArray<TWeakObjectPtr<UActorIOAction>> GetInputActionsForObject(AActor* InObject);

	/** @return Number of I/O actions currently loaded in the world that are targeting the given actor. */
	static int32 GetNumInputActionsForObject(AActor* InObject);

private:

	/**
	 * Exposes events from base Unreal Engine classes to the I/O system.
	 * Used to avoid the need of subclassing these base classes in order to expose them.
	 * Called automatically when collecting list of registered I/O functions for the given actor.
	 */
	static void GetNativeEventsForObject(AActor* InObject, FActorIOEventList& RegisteredEvents);

	/**
	 * Exposes functions from base Unreal Engine classes to the I/O system.
	 * Used to avoid the need of subclassing these base classes in order to expose them.
	 * Called automatically when collecting list of registered I/O functions for the given actor.
	 */
	static void GetNativeFunctionsForObject(AActor* InObject, FActorIOFunctionList& RegisteredFunctions);

	/** Event processor for the 'OnActorBeginOverlap' and 'OnActorEndOverlap' events. */
	UFUNCTION()
	void ProcessEvent_OnActorOverlap(AActor* OverlappedActor, AActor* OtherActor);

public:

	//#TODO: Rework? Custom nodes?

	UFUNCTION(BlueprintCallable, Category = "Actor IO", DisplayName = "Register I/O Event", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "DisplayName,TooltipText"))
	static void RegisterIOEvent(UObject* WorldContextObject, UPARAM(Ref) TArray<FActorIOEvent>& RegisterTo, FName EventId, const FText& DisplayName, const FText& TooltipText, FName EventDispatcherName);

	UFUNCTION(BlueprintCallable, Category = "Actor IO", DisplayName = "Register I/O Function", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "DisplayName,TooltipText", AdvancedDisplay = "SubobjectName"))
	static void RegisterIOFunction(UObject* WorldContextObject, UPARAM(Ref) TArray<FActorIOFunction>& RegisterTo, FName FunctionId, const FText& DisplayName, const FText& TooltipText, FString FunctionToExec, FName SubobjectName);
};

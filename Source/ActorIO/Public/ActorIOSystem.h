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
	static void GetNativeEventsForObject(AActor* InObject, FActorIOEventList& EventRegistry);

	/**
	 * Exposes functions from base Unreal Engine classes to the I/O system.
	 * Used to avoid the need of subclassing these base classes in order to expose them.
	 * Called automatically when collecting list of registered I/O functions for the given actor.
	 */
	static void GetNativeFunctionsForObject(AActor* InObject, FActorIOFunctionList& FunctionRegistry);

	/** Event processor for the 'OnActorBeginOverlap' and 'OnActorEndOverlap' events. */
	UFUNCTION()
	void ProcessEvent_OnActorOverlap(AActor* OverlappedActor, AActor* OtherActor);

public:

	/**
	 * Add a new I/O event to the actor's event list.
	 * Use this to expose a blueprint event dispatcher to the I/O system.
	 * This function should only be called when the I/O interface is registering events to an actor.
	 * 
	 * @param WorldContextObject Reference to the object where this function is being called.
	 * @param Registry The list of I/O events we are adding to.
	 * @param EventId Unique id of the event. Recommended format is ClassName::EventName.
	 * @param DisplayNameText Display name to use in the editor for this event.
	 * @param TooltipText Tooltip to use in the editor for this event.
	 * @param EventDispatcherName Name of the event dispatcher that should be exposed.
	 * @param EventProcessorName Name of a function that should be called when firing this event. Use this to handle named arguments (params) for this event. 
	 */
	UFUNCTION(BlueprintCallable, Category = "Actor IO", DisplayName = "Register I/O Event", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "DisplayNameText,TooltipText", AdvancedDisplay = "EventProcessorName", Keywords = "IO"))
	static void RegisterIOEvent(UObject* WorldContextObject, UPARAM(Ref) FActorIOEventList& Registry, FName EventId, const FText& DisplayNameText, const FText& TooltipText, FName EventDispatcherName, FName EventProcessorName);

	/**
	 * Add a new I/O function to the actor's function list.
	 * Use this to expose a blueprint function to the I/O system.
	 * This function should only be called when the I/O interface is registering functions to an actor.
	 *
	 * @param WorldContextObject Reference to the object where this function is being called.
	 * @param Registry The list of I/O functions we are adding to.
	 * @param FunctionId Unique id of the function. Recommended format is ClassName::FunctionName.
	 * @param DisplayNameText Display name to use in the editor for this function.
	 * @param TooltipText Tooltip to use in the editor for this function.
	 * @param FunctionToExec Name of the blueprint function that should be exposed.
	 * @param SubobjectName Specific subobject to call the function on instead of the actor itself.
	 */
	UFUNCTION(BlueprintCallable, Category = "Actor IO", DisplayName = "Register I/O Function", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "DisplayNameText,TooltipText", AdvancedDisplay = "SubobjectName", Keywords = "IO"))
	static void RegisterIOFunction(UObject* WorldContextObject, UPARAM(Ref) FActorIOFunctionList& Registry, FName FunctionId, const FText& DisplayNameText, const FText& TooltipText, FString FunctionToExec, FName SubobjectName);
};

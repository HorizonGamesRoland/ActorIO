// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "UObject/Interface.h"
#include "ActorIOInterface.generated.h"

/**
 * Interface for an actor exposing events and functions to the I/O system.
 */
UINTERFACE(MinimalAPI, Blueprintable, DisplayName = "Actor I/O Interface")
class UActorIOInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * API for an actor exposing events and functions to the I/O system.
 */
class ACTORIO_API IActorIOInterface
{
	GENERATED_BODY()

public:

	/**
	 * Called when building list of registered I/O events for the actor.
	 * This function is called every time the list of registered events is requested for the actor.
	 * Called in editor and at runtime!
	 */
	virtual void RegisterIOEvents(FActorIOEventList& EventRegistry) = 0;

	/**
	 * Called when building list of registered I/O functions for the actor.
	 * This function is called every time the list of registered functions is requested for the actor.
	 * Called in editor and at runtime!
	 */
	virtual void RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry) = 0;

	/**
	 * Event when building list of registered I/O events for the actor.
	 * 
	 * @param EventRegistry Current list of registered events.
	 */
	UFUNCTION(BlueprintImplementableEvent, CallInEditor, Category = "Actor IO", DisplayName = "Register I/O Events", meta = (ForceAsFunction, Keywords = "IO"))
	void K2_RegisterIOEvents(UPARAM(Ref) FActorIOEventList& EventRegistry);

	/**
	 * Event when building list of registered I/O functions for the actor.
	 *
	 * @param FunctionRegistry Current list of registered functions.
	 */
	UFUNCTION(BlueprintImplementableEvent, CallInEditor, Category = "Actor IO", DisplayName = "Register I/O Functions", meta = (ForceAsFunction, Keywords = "IO"))
	void K2_RegisterIOFunctions(UPARAM(Ref) FActorIOFunctionList& FunctionRegistry);

	/**
	 * Opportunity to add locally available named arguments to the current execution context.
	 * These named arguments are available for the implementing actor only.
	 * Called at runtime, when executing an I/O action.
	 */
	virtual void GetLocalNamedArguments(FActionExecutionContext& ExecutionContext) {}

	/**
	 * Opportunity for blueprints to add locally available named arguments to the current execution context.
	 * These named arguments are available for the implementing actor only.
	 * Called at runtime, before executing an I/O action.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Actor IO", DisplayName = "Get Local Named Arguments", meta = (ForceAsFunction, Keywords = "IO"))
	void K2_GetLocalNamedArguments();

	/**
	 * Called before the actor executes one of its output I/O actions.
	 * Use this to abort action execution as needed.
	 * If this function returns true then the execution will be aborted.
	 * 
	 * @param Action The action that is being executed.
	 * @return Whether the action should be aborted.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Actor IO", DisplayName = "Conditional Abort I/O Action", meta = (Keywords = "IO"))
	bool ConditionalAbortIOAction(UActorIOAction* Action);
	virtual bool ConditionalAbortIOAction_Implementation(UActorIOAction* Action) { return false; }
};
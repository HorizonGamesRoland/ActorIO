// Copyright 2025 Horizon Games. All Rights Reserved.

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

public:

	/**
	 * Determine whether the action can be executed or not.
	 * If this function returns false then the action will not be executed.
	 */
	virtual bool CanExecuteAction(UActorIOAction* Action) { return true; }

	/**
	 * Determine whether the action can be executed or not.
	 * If this function returns false then the action will not be executed.
	 * 
	 * @return Whether to execute the action or not.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Actor IO", DisplayName = "Can Execute Action")
	bool K2_CanExecuteAction(UActorIOAction* Action) const;
};
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
	 * Called when the actor is executing one of its output I/O actions.
	 * Use this to abort action execution as needed.
	 * If this function returns false then the action will not be executed.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Actor IO", DisplayName = "On Executing I/O Action")
	bool OnExecutingIOAction(UActorIOAction* Action);
	virtual bool OnExecutingIOAction_Implementation(UActorIOAction* Action) { return true; }
};
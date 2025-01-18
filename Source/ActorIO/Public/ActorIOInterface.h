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

	// #TODO: Rework so that it doesn't require the list params?

	/**
	 * Event when building list of registered I/O events for the actor.
	 * 
	 * @param EventRegistry Current list of registered events.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, CallInEditor, Category = "Actor IO", DisplayName = "Register I/O Events", meta = (ForceAsFunction))
	void RegisterIOEvents(UPARAM(Ref) FActorIOEventList& EventRegistry);
	virtual void RegisterIOEvents_Implementation(FActorIOEventList& EventRegistry) {}

	/**
	 * Event when building list of registered I/O functions for the actor.
	 *
	 * @param FunctionRegistry Current list of registered functions.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, CallInEditor, Category = "Actor IO", DisplayName = "Register I/O Functions", meta = (ForceAsFunction))
	void RegisterIOFunctions(UPARAM(Ref) FActorIOFunctionList& FunctionRegistry);
	virtual void RegisterIOFunctions_Implementation(FActorIOFunctionList& FunctionRegistry) {}
};
// Copyright 2025 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "Subsystems/WorldSubsystem.h"
#include "ActorIOSubsystemBase.generated.h"

class UActorIOAction;

/**
 * Base implementation of the Actor I/O Subsystem.
 * Stores the current action execution context, and exposes native I/O events and functions.
 */
UCLASS()
class ACTORIO_API UActorIOSubsystemBase : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UActorIOSubsystemBase();

public:

	/**
	 * The current I/O action execution context.
	 * Only valid at runtime between an actor receiving the execute action signal, and sending the command to the target actor.
	 * Use FActionExecutionContext::Get() to access unless you have direct reference to the I/O system.
	 */
	UPROPERTY(Transient)
	FActionExecutionContext ActionExecContext;

public:

	/**
	 * Exposes events from base Unreal Engine classes to the I/O system.
	 * Used to avoid the need of subclassing these base classes in order to expose them.
	 * Called in editor and at runtime, when registering I/O events.
	 */
	virtual void GetNativeEventsForObject(AActor* InObject, FActorIOEventList& EventRegistry);

	/**
	 * Exposes functions from base Unreal Engine classes to the I/O system.
	 * Used to avoid the need of subclassing these base classes in order to expose them.
	 * Called in editor and at runtime, when registering I/O functions.
	 */
	virtual void GetNativeFunctionsForObject(AActor* InObject, FActorIOFunctionList& FunctionRegistry);

	/**
	 * Opportunity to add globally available named arguments to the current execution context.
	 * Think stuff like reference to player character, or player controller.
	 * Called at runtime, when executing an I/O action.
	 */
	virtual void SetGlobalNamedArguments(FActionExecutionContext& ExecutionContext);

private:

	/** Event processor for the 'OnActorBeginOverlap' and 'OnActorEndOverlap' events. */
	UFUNCTION()
	void ProcessEvent_OnActorOverlap(AActor* OverlappedActor, AActor* OtherActor);

	/** Event processor for the 'OnDestroyed' event of actors. */
	UFUNCTION()
	void ProcessEvent_OnActorDestroyed(AActor* DestroyedActor);
};

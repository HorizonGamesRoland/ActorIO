// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "Subsystems/WorldSubsystem.h"
#include "ActorIOSubsystemBase.generated.h"

class UActorIOAction;

/**
 * Base implementation of the Actor I/O Subsystem.
 * Stores the current action execution context, and exposes native I/O events and functions.
 */
UCLASS(Blueprintable)
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

	/** Get the I/O subsystem of the given world. */
	static UActorIOSubsystemBase* Get(UObject* WorldContextObject);

public:

	/**
	 * Invokes a function on the target object with parameters using the reflection system.
	 * 
	 * @param Target Object to call the function on.
	 * @param Str The command to execute. Contains function name followed by the params, delimited by whitespaces.
	 * @param Ar Output device to push error messages to. Use FOutputDeviceNull if not needed.
	 * @param Executor Object that is executing the function.
	 */
	virtual bool ExecuteCommand(UObject* Target, const TCHAR* Str, FOutputDevice& Ar, UObject* Executor);

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
	virtual void GetGlobalNamedArguments(FActionExecutionContext& ExecutionContext);

	/**
	 * Opportunity for blueprint layer to add globally available named arguments to the current execution context.
	 * Think stuff like reference to player character, or player controller.
	 * Called at runtime, before executing an I/O action.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Actor IO", DisplayName = "Get Global Named Arguments", meta = (ForceAsFunction, Keywords = "IO"))
	void K2_GetGlobalNamedArguments();

private:

	/** Event processor for the 'OnActorBeginOverlap' and 'OnActorEndOverlap' events. */
	UFUNCTION()
	void ProcessEvent_OnActorOverlap(AActor* OverlappedActor, AActor* OtherActor);

	/** Event processor for the 'OnDestroyed' event of actors. */
	UFUNCTION()
	void ProcessEvent_OnActorDestroyed(AActor* DestroyedActor);

public:

	//~ Begin UWorldSubsystem Interface
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override final;
	//~ End UWorldSubsystem Interface
};

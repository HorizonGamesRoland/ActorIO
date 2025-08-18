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
	 * Opportunity to externally expose events of an actor to the I/O system.
	 * Used to expose functionality from base Unreal Engine classes without the need to subclass them.
	 * Called in editor and at runtime, when registering I/O events.
	 */
	virtual void RegisterNativeEventsForObject(AActor* InObject, FActorIOEventList& EventRegistry);

	/**
	 * Opportunity for blueprints to externally expose events of an actor to the I/O system.
	 * Called in editor and at runtime, when registering I/O events.
	 */
	UFUNCTION(BlueprintImplementableEvent, CallInEditor, Category = "Actor IO", DisplayName = "Register Native Events For Object", meta = (ForceAsFunction, Keywords = "IO"))
	void K2_RegisterNativeEventsForObject(AActor* InObject, UPARAM(Ref) FActorIOEventList& EventRegistry);

	/**
	 * Opportunity to externally expose functions of an actor to the I/O system.
	 * Used to expose functionality from base Unreal Engine classes without the need to subclass them.
	 * Called in editor and at runtime, when registering I/O events.
	 */
	virtual void RegisterNativeFunctionsForObject(AActor* InObject, FActorIOFunctionList& FunctionRegistry);

	/**
	 * Opportunity for blueprints to externally expose functions of an actor to the I/O system.
	 * Called in editor and at runtime, when registering I/O events.
	 */
	UFUNCTION(BlueprintImplementableEvent, CallInEditor, Category = "Actor IO", DisplayName = "Register Native Functions For Object", meta = (ForceAsFunction, Keywords = "IO"))
	void K2_RegisterNativeFunctionsForObject(AActor* InObject, UPARAM(Ref) FActorIOFunctionList& FunctionRegistry);

	/**
	 * Opportunity to add globally available named arguments to the current execution context.
	 * These named arguments are available for all actors.
	 * Called at runtime, when executing an I/O action.
	 */
	virtual void GetGlobalNamedArguments(FActionExecutionContext& ExecutionContext);

	/**
	 * Opportunity for blueprints to add globally available named arguments to the current execution context.
	 * These named arguments are available for all actors.
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

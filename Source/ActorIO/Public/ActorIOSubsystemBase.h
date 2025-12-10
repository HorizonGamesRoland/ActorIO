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
class ACTORIO_API UActorIOSubsystemBase : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UActorIOSubsystemBase();

protected:

	/**
	 * The current I/O action execution context.
	 * Only valid at runtime between an actor receiving the execute action signal, and queuing the I/O message.
	 * Use FActionExecutionContext::Get() to access unless you have direct reference to the I/O system.
	 */
	UPROPERTY(Transient)
	FActionExecutionContext ActionExecContext;

	/**
	 * List of I/O messages that are queued for delivery.
	 * Each message contains a formatted UnrealScript command that will be sent to the message target.
	 */
	UPROPERTY(Transient)
	TArray<FActorIOMessage> PendingMessages;

public:

	/** Get the I/O subsystem of the given world. */
	static UActorIOSubsystemBase* Get(UObject* WorldContextObject);

	/**
	 * Opportunity to externally expose events of an actor to the I/O system.
	 * Used to expose functionality from base Unreal Engine classes without the need to subclass them.
	 * Called in editor and at runtime, when registering I/O events.
	 */
	virtual void RegisterNativeEventsForObject(AActor* InObject, FActorIOEventList& EventRegistry);

	/**
	 * Opportunity to externally expose functions of an actor to the I/O system.
	 * Used to expose functionality from base Unreal Engine classes without the need to subclass them.
	 * Called in editor and at runtime, when registering I/O events.
	 */
	virtual void RegisterNativeFunctionsForObject(AActor* InObject, FActorIOFunctionList& FunctionRegistry);

	/**
	 * Opportunity to add globally available named arguments to the current execution context.
	 * These named arguments are available for all actors.
	 * Called at runtime, when executing an I/O action.
	 */
	virtual void GetGlobalNamedArguments(FActionExecutionContext& ExecutionContext);

	/**
	 * Opportunity for blueprints to externally expose events of an actor to the I/O system.
	 * Called in editor and at runtime, when registering I/O events.
	 */
	UFUNCTION(BlueprintImplementableEvent, CallInEditor, Category = "Actor IO", DisplayName = "Register Native Events For Object", meta = (ForceAsFunction, Keywords = "IO"))
	void K2_RegisterNativeEventsForObject(AActor* InObject, UPARAM(Ref) FActorIOEventList& EventRegistry);

	/**
	 * Opportunity for blueprints to externally expose functions of an actor to the I/O system.
	 * Called in editor and at runtime, when registering I/O events.
	 */
	UFUNCTION(BlueprintImplementableEvent, CallInEditor, Category = "Actor IO", DisplayName = "Register Native Functions For Object", meta = (ForceAsFunction, Keywords = "IO"))
	void K2_RegisterNativeFunctionsForObject(AActor* InObject, UPARAM(Ref) FActorIOFunctionList& FunctionRegistry);

	/**
	 * Opportunity for blueprints to add globally available named arguments to the current execution context.
	 * These named arguments are available for all actors.
	 * Called at runtime, before executing an I/O action.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Actor IO", DisplayName = "Get Global Named Arguments", meta = (ForceAsFunction, Keywords = "IO"))
	void K2_GetGlobalNamedArguments();

public:

	/**
	 * Queue an I/O message with a formatted UnrealScript command to be delivered to the target object.
	 * Message delivery can be delayed.
	 */
	virtual void SendMessage(UObject* Sender, UObject* Target, FString& Command, float Delay = 0.0f);

	/** @return List of I/O messages that are queued for delivery. */
	const TArray<FActorIOMessage>& GetPendingMessages() const { return PendingMessages; }

	/**
	 * Executes an UnrealScript command on the target object.
	 * The command will invoke a function (optionally with parameters) using the C++ reflection system.
	 *
	 * @param Target Object to execute the command on.
	 * @param Str The command to execute. Contains function name followed by the params, delimited by whitespaces.
	 * @param Ar Output device to push error messages to. Use FOutputDeviceNull if not needed.
	 * @param Executor Object that is executing the function.
	 */
	virtual bool ExecuteCommand(UObject* Target, const TCHAR* Str, FOutputDevice& Ar, UObject* Executor);

	/** @return Reference to the global execution context. */
	FActionExecutionContext& GetExecutionContext() { return ActionExecContext; }

protected:

	/** Handles the delivery of an I/O message. */
	virtual void ProcessMessage(FActorIOMessage& InMessage);

private:

	/** Event processor for the 'OnActorBeginOverlap' and 'OnActorEndOverlap' events. */
	UFUNCTION()
	void ProcessEvent_OnActorOverlap(AActor* OverlappedActor, AActor* OtherActor);

	/** Event processor for the 'OnDestroyed' event of actors. */
	UFUNCTION()
	void ProcessEvent_OnActorDestroyed(AActor* Actor, EEndPlayReason::Type EndPlayReason);

public:

	//~ Begin UTickableWorldSubsystem Interface
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override final;
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	//~ End UTickableWorldSubsystem Interface
};

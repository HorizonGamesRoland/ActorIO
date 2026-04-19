// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "Subsystems/WorldSubsystem.h"
#include "ActorIOSubsystemBase.generated.h"

class UActorIOAction;

/**
 * Base implementation of the Actor I/O Subsystem.
 * This subsystem handles the delivery and execution of I/O messages.
 * It is also responsible for exposing native events and functions to the I/O system.
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
	 * List of levels that are considered for delivering I/O messages.
	 * This abstraction ensures we do not execute I/O actions before their state can be restored from a save file.
	 */
	TArray<TWeakObjectPtr<ULevel>> ActiveLevels;

	/**
	 * List of I/O messages that are queued for delivery.
	 * Each message contains a formatted UnrealScript command that will be sent to the message target.
	 */
	TArray<FActorIOMessage> PendingMessages;

	/**
	 * The current I/O action execution context.
	 * Only valid at runtime between an actor receiving the execute action signal, and queuing the I/O message.
	 * Use FActionExecutionContext::Get() to access.
	 */
	UPROPERTY(Transient)
	FActionExecutionContext ActionExecContext;

	/** Handle for when a level is added to the world. */
	FDelegateHandle DelegateHandle_OnLevelAdded;

	/** Handle for when a level is removed from the world. */
	FDelegateHandle DelegateHandle_OnLevelRemoved;

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
	 * Add a level to the list of 'active levels' so that it is now considered for I/O message delivery.
	 * It also triggers the execution of all pending messages who's participants were awaiting this level's activation.
	 * This abstraction ensures we do not execute I/O actions before their state can be restored from a save file.
	 */
	UFUNCTION(BlueprintCallable, Category = "ActorIO")
	void ActivateLevel(ULevel* InLevel);
	
	/**
	 * Remove a level from the list of 'active levels' so that it is now longer considered for I/O message delivery.
	 * Messages from inactive levels are put into a pending list, and delivered once the level is activated.
	 */
	UFUNCTION(BlueprintCallable, Category = "ActorIO")
	void DeactivateLevel(ULevel* InLevel, bool bRemoveMessages = true);

	/**
	 * Remove all invald (nullptr) elements from the 'active levels' list.
	 * Called automatically before a level is added or removed from the list.
	 */
	UFUNCTION(BlueprintCallable, Category = "ActorIO")
	void CompactActiveLevels();

	/** Get whether the level is part of the 'active levels' list. */
	UFUNCTION(BlueprintPure, Category = "ActorIO")
	bool IsLevelActive(ULevel* InLevel) const;

	/** Get whether the level is part of the 'active levels' list */
	UFUNCTION(BlueprintPure, Category = "ActorIO")
	bool IsLevelActiveByPath(const FSoftObjectPath& InLevelPath) const;

	/** @return List of 'active levels' that are considered for I/O message delivery. */
	const TArray<TWeakObjectPtr<ULevel>>& GetActiveLevels() const { return ActiveLevels; }

	/**
	 * Blueprint access to the list of 'active levels' that are considered for I/O message delivery.
	 * This only returns valid ULevel references.
	 * The actual list may have additional (nullptr) elements awaiting removal.
	 */
	UFUNCTION(BlueprintPure, Category = "ActorIO", DisplayName = "Get Active Levels")
	TArray<ULevel*> K2_GetActiveLevels() const;
	
	/** Get the number of 'active levels' that are considered for I/O message delivery. */
	UFUNCTION(BlueprintPure, Category = "ActorIO")
	int32 GetNumActiveLevels() const { return ActiveLevels.Num(); }

	/** Get the path to the ULevel that contains the given object. */
	UFUNCTION(BlueprintPure, Category = "ActorIO")
	FSoftObjectPath GetLevelPathFromObjectPath(const FSoftObjectPath& InObjectPath) const;

public:

	/**
	 * Queue an I/O message with a formatted UnrealScript command to be delivered to the target object.
	 * Message delivery can be delayed.
	 */
	virtual void QueueMessage(FActorIOMessage& InMessage);

	/**
	 * Remove all pending messages that were sent by the given I/O action.
	 * This is used when an I/O action's state is restored from a save file and the action needs to recall its messages.
	 */
	void RemovePendingMessages(UActorIOAction* InAction);

	/** Remove all pending messages that were sent from the given level. */
	void RemovePendingMessages(ULevel* InLevel);

	/** @return List of I/O messages that are queued for delivery. */
	const TArray<FActorIOMessage>& GetPendingMessages() const { return PendingMessages; }

	/** @return Number of I/O messages that are queued for delivery. */
	int32 GetNumPendingMessages() const { return PendingMessages.Num(); }

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

public:

	/**
	 * Serialize the I/O subsystem into raw data that can be stored in save files easily.
	 * Use in conjunction with RestoreFromRawData.
	 */
	UFUNCTION(BlueprintCallable, Category = "ActorIO", meta = (Keywords = "Save,Load"))
	void SerializeToRawData(TArray<uint8>& RawData);

	/**
	 * Serialize the I/O subsystem back from previously saved raw data, restoring the saved state.
	 * Use in conjunction with SerializeToRawData.
	 */
	UFUNCTION(BlueprintCallable, Category = "ActorIO", meta = (Keywords = "Save,Load"))
	void RestoreFromRawData(UPARAM(Ref) TArray<uint8>& RawData);

protected:

	/** Update remaining time on all pending messages, potentially activating them. */
	void TickPendingMessages(float DeltaTime);
	
	/** Determine if the message can be ticked. Not const because we want to update inner properties. */
	bool PreTickMessage(FActorIOMessage& InMessage, bool bInitialTick = false);

	/** Handles the delivery of an I/O message. */
	virtual void ProcessMessage(const FActorIOMessage& InMessage);

	/** Callback for when a level is added to the world. */
	void OnLevelAddedToWorld(ULevel* InLevel, UWorld* InWorld);

	/** Callback for when a level is removed from the world. */
	void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld);

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
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	virtual void Serialize(FStructuredArchive::FRecord Record) override;
	//~ End UTickableWorldSubsystem Interface
};

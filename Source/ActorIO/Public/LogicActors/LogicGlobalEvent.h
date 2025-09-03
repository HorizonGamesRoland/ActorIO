// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicGlobalEvent.generated.h"

struct FActorsInitializedParams;

/**
 * An actor that listens for global world events.
 */
UCLASS()
class ACTORIO_API ALogicGlobalEvent : public ALogicActorBase
{
    GENERATED_BODY()

public:

    /** Default constructor. */
    ALogicGlobalEvent();

public:

    /** Event when the game world is initialized. Called after all actors in the persistent level have been initialized, but before 'BeginPlay'. */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FSimpleActionDelegate OnWorldInitialized;

    /** Event when 'BeginPlay' is called for this actor. */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FSimpleActionDelegate OnActorBeginPlay;

    /** Event when 'EndPlay' is called for this actor. */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FSimpleActionDelegate OnActorEndPlay;

    /** Event when the game world is being torn down. This means we are leaving the map. Called before 'EndPlay' is dispatched to all actors. */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FSimpleActionDelegate OnWorldTeardown;

protected:

    //~ Begin ALogicActorBase Interface
    virtual void RegisterIOEvents(FActorIOEventList& EventRegistry) override;
    virtual void RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry) override;
    virtual void PostInitializeComponents() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    //~ End ALogicActorBase Interface

public:

    /**
     * Call a function on the level blueprint actor of the level.
     * Can pass in function params in following format: "FuncName Param1 Param2 Param3 ..."
     */
    UFUNCTION(BlueprintCallable, Category = "LogicActors|LogicGlobalEvent")
    void CallLevelBlueprintFunction(FString Command);

protected:

    /** Called when the world is initialized. */
    void OnWorldInitializedCallback(const FActorsInitializedParams& ActorInitParams);

    /** Called when the world is being torn down (unloaded). */
    void OnWorldTeardownCallback(UWorld* InWorld);
};
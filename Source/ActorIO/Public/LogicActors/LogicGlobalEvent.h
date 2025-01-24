// Copyright 2025 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicGlobalEvent.generated.h"

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

    /** Event when the world is initialized. Called after all actors have been initialized, but before 'BeginPlay'. */
    UPROPERTY(BlueprintAssignable, Category = "Global Event")
    FSimpleActionDelegate OnWorldInitialized;

    /** Event when 'BeginPlay' is called for this actor. */
    UPROPERTY(BlueprintAssignable, Category = "Global Event")
    FSimpleActionDelegate OnBeginPlay;

    /** Event when the world is being torn down. This means we are leaving the map. Called before 'EndPlay' is dispatched to all actors. */
    UPROPERTY(BlueprintAssignable, Category = "Global Event")
    FSimpleActionDelegate OnWorldTeardown;

protected:

    //~ Begin ALogicActorBase Interface
    virtual void RegisterIOEvents(FActorIOEventList& EventRegistry) override;
    virtual void RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry) override;
    virtual void PostInitializeComponents() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    //~ End ALogicActorBase Interface

protected:

    /** Called when the world is initialized. */
    void OnWorldInitializedCallback(const FActorsInitializedParams& ActorInitParams);

    /** Called when the world is being torn down (unloaded). */
    void OnWorldTeardownCallback(UWorld* World);
};
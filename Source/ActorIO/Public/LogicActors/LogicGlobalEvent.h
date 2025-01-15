// Copyright 2025 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicGlobalEvent.generated.h"

UCLASS()
class ACTORIO_API ALogicGlobalEvent : public ALogicActorBase
{
    GENERATED_BODY()

public:

    ALogicGlobalEvent();

public:

    UPROPERTY(BlueprintAssignable, Category = "Global Event")
    FSimpleActionDelegate OnWorldInitialized;

    UPROPERTY(BlueprintAssignable, Category = "Global Event")
    FSimpleActionDelegate OnBeginPlay;

    UPROPERTY(BlueprintAssignable, Category = "Global Event")
    FSimpleActionDelegate OnWorldTeardown;

public:

    virtual void PostInitializeComponents() override;
    virtual void RegisterIOEvents_Implementation(FActorIOEventList& RegisteredEvents) override;
    virtual void RegisterIOFunctions_Implementation(FActorIOFunctionList& RegisteredFunctions) override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:

    void OnWorldInitializedCallback(const FActorsInitializedParams& ActorInitParams);

    void OnWorldTeardownCallback(UWorld* World);
};
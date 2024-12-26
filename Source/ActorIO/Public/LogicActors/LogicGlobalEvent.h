// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicGlobalEvent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGlobalEvent);

UCLASS()
class ACTORIO_API ALogicGlobalEvent : public ALogicActorBase
{
    GENERATED_BODY()

public:

    ALogicGlobalEvent();

protected:

    FOnGlobalEvent OnWorldInitialized;

    FOnGlobalEvent OnBeginPlay;

    FOnGlobalEvent OnWorldTeardown;

public:

    virtual void PostInitializeComponents() override;
    virtual void RegisterIOEvents_Implementation(TArray<FActorIOEvent>& RegisteredEvents) override;
    virtual void RegisterIOFunctions_Implementation(TArray<FActorIOFunction>& RegisteredFunctions) override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

    FOnGlobalEvent& GetOnWorldInitialized() { return OnWorldInitialized; }

    FOnGlobalEvent& GetOnBeginPlay() { return OnBeginPlay; }

    FOnGlobalEvent& GetOnWorldTeardown() { return OnWorldTeardown; }

protected:

    void OnWorldInitializedCallback(const FActorsInitializedParams& ActorInitParams);

    void OnWorldTeardownCallback(UWorld* World);
};
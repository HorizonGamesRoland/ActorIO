// Copyright 2025 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicRelay.generated.h"

/**
 * An actor that forwards outputs to other actors.
 * Use this to fire many outputs from a single output, or to break an I/O chain.
 * It can also be very helpful for organization.
 */
UCLASS()
class ACTORIO_API ALogicRelay : public ALogicActorBase
{
    GENERATED_BODY()

public:

    /** Default constructor. */
    ALogicRelay();

public:

    // #TODO: Add call on server to this?

    /** Whether the relay is enabled or not. */
    UPROPERTY(EditInstanceOnly, Category = "Relay")
    bool bIsEnabled;

    /** Event when the relay is triggered */
    UPROPERTY(BlueprintAssignable, Category = "Relay")
    FSimpleActionDelegate OnTrigger;

public:

    //~ Begin ALogicActorBase Interface
    virtual void RegisterIOEvents(FActorIOEventList& EventRegistry) override;
    virtual void RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry) override;
    //~ End ALogicActorBase Interface

public:

    /** Trigger the relay, causing the 'OnTrigger' event to fire if enabled. */
    UFUNCTION(BlueprintCallable, Category = "Relay")
    void Trigger();

    /** Allow the relay to fire the 'OnTrigger' event. */
    UFUNCTION(BlueprintCallable, Category = "Relay")
    void Enable();

    /** Prevent the relay from firing the 'OnTrigger' event. */
    UFUNCTION(BlueprintCallable, Category = "Relay")
    void Disable();

    /** Get whether the relay is currently enabled or not. */
    UFUNCTION(BlueprintPure, Category = "Relay")
    bool IsEnabled() const;
};
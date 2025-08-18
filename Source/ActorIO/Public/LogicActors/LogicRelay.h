// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

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

    /** Whether the relay is enabled or not. */
    UPROPERTY(EditInstanceOnly, Category = "Relay")
    bool bIsEnabled;

    /** Event when the relay is triggered */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FSimpleActionDelegate OnTrigger;

protected:

    //~ Begin ALogicActorBase Interface
    virtual void RegisterIOEvents(FActorIOEventList& EventRegistry) override;
    virtual void RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry) override;
    //~ End ALogicActorBase Interface

public:

    /** Trigger the relay, causing the 'OnTrigger' event to fire if enabled. */
    UFUNCTION(BlueprintCallable, Category = "LogicActors|LogicRelay")
    void Trigger();

    /** Allow the relay to fire the 'OnTrigger' event. */
    UFUNCTION(BlueprintCallable, Category = "LogicActors|LogicRelay")
    void Enable();

    /** Prevent the relay from firing the 'OnTrigger' event. */
    UFUNCTION(BlueprintCallable, Category = "LogicActors|LogicRelay")
    void Disable();

    /** Get whether the relay is currently enabled or not. */
    UFUNCTION(BlueprintPure, Category = "LogicActors|LogicRelay")
    bool IsEnabled() const;
};
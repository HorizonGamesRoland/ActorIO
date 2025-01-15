// Copyright 2025 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicCounter.generated.h"

/** Delegate with the counter's current value. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCounterValue, int32, Value);

/**
 * An actor that stores and manipulates a numeric value.
 * Use this to count how many times certain things happen in the game.
 */
UCLASS()
class ACTORIO_API ALogicCounter : public ALogicActorBase
{
    GENERATED_BODY()

public:

    /** Default constructor. */
    ALogicCounter();

public:

    /** Default value of the counter. */
    UPROPERTY(EditInstanceOnly, Category = "Counter")
    int32 InitialValue;

    /**
     * Optional target value for the counter to reach.
     * When counter reaches this value, the 'OnTargetValueReached' output event is fired.
     * The counter continues to work after the target value is reached.
     */
    UPROPERTY(EditInstanceOnly, Category = "Counter")
    int32 TargetValue;

    /** Whether to clamp the counter current value between 0 and the target value. */
    UPROPERTY(EditInstanceOnly, Category = "Counter")
    bool bClampValue;

    /** Event when the current value is changed. */
    UPROPERTY(BlueprintAssignable, Category = "Counter")
    FOnCounterValue OnValueChanged;

    /** Event when the current value equals or greater then the target value. */
    UPROPERTY(BlueprintAssignable, Category = "Counter")
    FOnCounterValue OnTargetValueReached;

    /** Event when the current value is read using the 'GetValue' function. */
    UPROPERTY(BlueprintAssignable, Category = "Counter")
    FOnCounterValue OnGetValue;

protected:

    /** Current value of the counter. */
    int32 CurrentValue;

public:

    //~ Begin ALogicActorBase Interface
    virtual void PostInitializeComponents() override;
    virtual void RegisterIOEvents_Implementation(FActorIOEventList& RegisteredEvents) override;
    virtual void RegisterIOFunctions_Implementation(FActorIOFunctionList& RegisteredFunctions) override;
    //~ End ALogicActorBase Interface

public:

    /** Add to the current value. */
    UFUNCTION(BlueprintCallable, Category = "Counter")
    void Add(int32 Amount = 1);

    /** Subtract from the current value. */
    UFUNCTION(BlueprintCallable, Category = "Counter")
    void Subtract(int32 Amount = 1);

    /** Set the current value directly. */
    UFUNCTION(BlueprintCallable, Category = "Counter")
    void SetValue(int32 Value);

    /** Set a new target value. Fires 'OnTargetValueReached' if current value equals or greater then the new target value. */
    UFUNCTION(BlueprintCallable, Category = "Counter")
    void SetTargetValue(int32 Value);

    /** Fire the 'OnGetValue' event with the current value. */
    UFUNCTION(BlueprintPure, Category = "Counter")
    int32 GetValue() const;

protected:

    /** Event processor for the 'OnGetValue' event. */
    UFUNCTION()
    void ProcessEvent_OnGetValue(int32 Value);
};
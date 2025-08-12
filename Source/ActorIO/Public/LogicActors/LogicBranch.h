// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicBranch.generated.h"

/**
 * An actor that tests a boolean value and fires the appropriate true or false output event.
 * Use this to branch between two potential outcomes.
 */
UCLASS()
class ACTORIO_API ALogicBranch : public ALogicActorBase
{
    GENERATED_BODY()

public:

    /** Default constructor. */
    ALogicBranch();

protected:

    //~ Begin ALogicActorBase Interface
    virtual void RegisterIOEvents(FActorIOEventList& EventRegistry) override;
    virtual void RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry) override;
    virtual void PostInitializeComponents() override;
    //~ End ALogicActorBase Interface

public:

    /** Default value of the branch. */
    UPROPERTY(EditInstanceOnly, Category = "Branch")
    bool bInitialValue;

    /** Event when the stored boolean value is true when 'Test' is called. */
    UPROPERTY(BlueprintAssignable, Category = "Branch")
    FSimpleActionDelegate OnTrue;

    /** Event when the stored boolean value is false when 'Test' is called. */
    UPROPERTY(BlueprintAssignable, Category = "Branch")
    FSimpleActionDelegate OnFalse;

protected:

    /** Current value of the branch. */
    UPROPERTY()
    bool bCurrentValue;

public:

    /** Set the boolean value without performing the comparison. Use this to hold a value for a future test. */
    UFUNCTION(BlueprintCallable, Category = "Branch")
    void SetValue(bool bValue);

    /** Set the boolean value and test it, firing 'OnTrue' or 'OnFalse' based on the new value. */
    UFUNCTION(BlueprintCallable, Category = "Branch")
    void SetValueAndTest(bool bValue);

    /** Toggle the boolean value between true and false without performing the comparison. */
    UFUNCTION(BlueprintCallable, Category = "Branch")
    void Toggle();

    /** Toggle the boolean value and test it, firing 'OnTrue' or 'OnFalse' based on the new value. */
    UFUNCTION(BlueprintCallable, Category = "Branch")
    void ToggleAndTest();

    /** Test the boolean value and fire 'OnTrue' or 'OnFalse' based on the value. */
    UFUNCTION(BlueprintCallable, Category = "Branch")
    void Test();

    /** Get the current boolean value of the branch. */
    UFUNCTION(BlueprintPure, Category = "Branch")
    bool GetValue() const { return bCurrentValue; }

protected:

    /** Event processor for the 'OnTrue' and 'OnFalse' events. */
    UFUNCTION()
    void ProcessEvent_OnTest();
};
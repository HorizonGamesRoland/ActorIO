// Copyright 2025 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicCompare.generated.h"

/**
 * An actor that compares two values and fires the appropriate output event.
 */
UCLASS()
class ACTORIO_API ALogicCompare : public ALogicActorBase
{
    GENERATED_BODY()

public:

    /** Default constructor. */
    ALogicCompare();

public:

    /** Default value to use. */
    UPROPERTY(EditInstanceOnly, Category = "Compare")
    FString InitialValue;

    /** The value to compare against. */
    UPROPERTY(EditInstanceOnly, Category = "Compare")
    FString CompareValue;

    /** Event when the current value equals the compare value. */
    UPROPERTY(BlueprintAssignable, Category = "Compare")
    FSimpleActionDelegate OnEquals;

    /** Event when the current value does not equal the compare value */
    UPROPERTY(BlueprintAssignable, Category = "Compare")
    FSimpleActionDelegate OnNotEquals;

    /** Event when the current value is less than the compare value. Only works with numeric values! */
    UPROPERTY(BlueprintAssignable, Category = "Compare")
    FSimpleActionDelegate OnLessThen;

    /** Event when the current value is greater than the compare value. Only works with numeric values! */
    UPROPERTY(BlueprintAssignable, Category = "Compare")
    FSimpleActionDelegate OnGreaterThen;

protected:

    /** Current value to use for comparison. */
    FString CurrentValue;

public:

    //~ Begin ALogicActorBase Interface
    virtual void PostInitializeComponents() override;
    virtual void RegisterIOEvents_Implementation(FActorIOEventList& EventRegistry) override;
    virtual void RegisterIOFunctions_Implementation(FActorIOFunctionList& FunctionRegistry) override;
    //~ End ALogicActorBase Interface

public:

    /** Set the current value without performing the comparison. */
    void SetValue(FString InValue);

    /** Set the current value and compare it against the compare value. */
    void SetValueAndCompare(FString InValue);

    /** Set the compare value. */
    void SetCompareValue(FString InValue);

    /** Compare the current value with the compare value. */
    void Compare();

protected:

    /** Event processor for the comparison events. */
    UFUNCTION()
    void ProcessEvent_OnCompare();
};
// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

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

    /** Maximum allowed difference before considering numeric values as equal. */
    UPROPERTY(EditInstanceOnly, Category = "Compare", meta = (ClampMin = "0"))
    float ErrorToleranceForNumericValues;

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
    UPROPERTY()
    FString CurrentValue;

protected:

    //~ Begin ALogicActorBase Interface
    virtual void RegisterIOEvents(FActorIOEventList& EventRegistry) override;
    virtual void RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry) override;
    virtual void GetLocalNamedArguments(FActionExecutionContext& ExecutionContext) override;
    virtual void PostInitializeComponents() override;
    //~ End ALogicActorBase Interface

public:

    /** Set the current value without performing the comparison. */
    UFUNCTION(BlueprintCallable, Category = "Compare")
    void SetValue(FString InValue);

    /** Set the current value and compare it against the compare value. */
    UFUNCTION(BlueprintCallable, Category = "Compare")
    void SetValueAndCompare(FString InValue);

    /** Set the compare value. */
    UFUNCTION(BlueprintCallable, Category = "Compare")
    void SetCompareValue(FString InValue);

    /** Compare the current value with the compare value. */
    UFUNCTION(BlueprintCallable, Category = "Compare")
    void Compare();
};
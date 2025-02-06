// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicCase.generated.h"

/**
 * An actor that compares the input value to up to 16 configured cases, firing the appropriate event for that case.
 */
UCLASS()
class ACTORIO_API ALogicCase : public ALogicActorBase
{
    GENERATED_BODY()

public:

    /** Default constructor. */
    ALogicCase();

public:

    /**
     * Case values to test against.
     * Max number of supported elements is 16.
     */
    UPROPERTY(EditInstanceOnly, Category = "Case")
    TArray<FString> Cases;

    /** Event when the input value equals the case value at index 0. */
    UPROPERTY(BlueprintAssignable, Category = "Case")
    FSimpleActionDelegate OnCase01;
    /** Event when the input value equals the case value at index 1. */
    UPROPERTY(BlueprintAssignable, Category = "Case")
    FSimpleActionDelegate OnCase02;
    /** Event when the input value equals the case value at index 2. */
    UPROPERTY(BlueprintAssignable, Category = "Case")
    FSimpleActionDelegate OnCase03;
    /** Event when the input value equals the case value at index 3. */
    UPROPERTY(BlueprintAssignable, Category = "Case")
    FSimpleActionDelegate OnCase04;
    /** Event when the input value equals the case value at index 4. */
    UPROPERTY(BlueprintAssignable, Category = "Case")
    FSimpleActionDelegate OnCase05;
    /** Event when the input value equals the case value at index 5. */
    UPROPERTY(BlueprintAssignable, Category = "Case")
    FSimpleActionDelegate OnCase06;
    /** Event when the input value equals the case value at index 6. */
    UPROPERTY(BlueprintAssignable, Category = "Case")
    FSimpleActionDelegate OnCase07;
    /** Event when the input value equals the case value at index 7. */
    UPROPERTY(BlueprintAssignable, Category = "Case")
    FSimpleActionDelegate OnCase08;
    /** Event when the input value equals the case value at index 8. */
    UPROPERTY(BlueprintAssignable, Category = "Case")
    FSimpleActionDelegate OnCase09;
    /** Event when the input value equals the case value at index 9. */
    UPROPERTY(BlueprintAssignable, Category = "Case")
    FSimpleActionDelegate OnCase10;
    /** Event when the input value equals the case value at index 10. */
    UPROPERTY(BlueprintAssignable, Category = "Case")
    FSimpleActionDelegate OnCase11;
    /** Event when the input value equals the case value at index 11. */
    UPROPERTY(BlueprintAssignable, Category = "Case")
    FSimpleActionDelegate OnCase12;
    /** Event when the input value equals the case value at index 12. */
    UPROPERTY(BlueprintAssignable, Category = "Case")
    FSimpleActionDelegate OnCase13;
    /** Event when the input value equals the case value at index 13. */
    UPROPERTY(BlueprintAssignable, Category = "Case")
    FSimpleActionDelegate OnCase14;
    /** Event when the input value equals the case value at index 14. */
    UPROPERTY(BlueprintAssignable, Category = "Case")
    FSimpleActionDelegate OnCase15;
    /** Event when the input value equals the case value at index 15. */
    UPROPERTY(BlueprintAssignable, Category = "Case")
    FSimpleActionDelegate OnCase16;

    /** Event when the input value does not equal any of the cases. */
    UPROPERTY(BlueprintAssignable, Category = "Case")
    FSimpleActionDelegate OnDefault;

protected:

    //~ Begin ALogicActorBase Interface
    virtual void RegisterIOEvents(FActorIOEventList& EventRegistry) override;
    virtual void RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry) override;
    //~ End ALogicActorBase Interface

public:

    /** Compare the value to the case values, and fire the appropriate 'OnCase' event. */
    void Test(FString InValue);
};
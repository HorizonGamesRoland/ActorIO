// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicCompare.generated.h"

UCLASS()
class ACTORIO_API ALogicCompare : public ALogicActorBase
{
    GENERATED_BODY()

public:

    ALogicCompare();

public:

    UPROPERTY(EditInstanceOnly, Category = "Compare")
    FString InitialValue;

    UPROPERTY(EditInstanceOnly, Category = "Compare")
    FString CompareValue;

    UPROPERTY(BlueprintAssignable, Category = "Compare")
    FSimpleActionDelegate OnEquals;

    UPROPERTY(BlueprintAssignable, Category = "Compare")
    FSimpleActionDelegate OnNotEquals;

    UPROPERTY(BlueprintAssignable, Category = "Compare")
    FSimpleActionDelegate OnLessThen;

    UPROPERTY(BlueprintAssignable, Category = "Compare")
    FSimpleActionDelegate OnGreaterThen;

protected:

    FString CurrentValue;

public:

    virtual void PostInitializeComponents() override;
    virtual void RegisterIOEvents_Implementation(FActorIOEventList& RegisteredEvents) override;
    virtual void RegisterIOFunctions_Implementation(FActorIOFunctionList& RegisteredFunctions) override;

public:

    void SetValue(FString InValue);

    void SetValueAndCompare(FString InValue);

    void SetCompareValue(FString InValue);

    void Compare();

protected:

    UFUNCTION()
    void ProcessEvent_OnCompare();
};
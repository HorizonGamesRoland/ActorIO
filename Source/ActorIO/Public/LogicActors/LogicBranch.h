// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicBranch.generated.h"

UCLASS()
class ACTORIO_API ALogicBranch : public ALogicActorBase
{
    GENERATED_BODY()

public:

    ALogicBranch();

public:

    virtual void PostInitializeComponents() override;
    virtual void RegisterIOEvents_Implementation(TArray<FActorIOEvent>& RegisteredEvents) override;
    virtual void RegisterIOFunctions_Implementation(TArray<FActorIOFunction>& RegisteredFunctions) override;

public:

    UPROPERTY(EditInstanceOnly, Category = "Branch")
    bool bInitialValue;

protected:

    bool bCurrentValue;

    FSimpleActionDelegate TrueEvent;

    FSimpleActionDelegate FalseEvent;

public:

    UFUNCTION(BlueprintCallable, Category = "Branch")
    void SetValue(bool bValue);

    UFUNCTION(BlueprintCallable, Category = "Branch")
    void SetValueAndTest(bool bValue);

    UFUNCTION(BlueprintCallable, Category = "Branch")
    void Toggle();

    UFUNCTION(BlueprintCallable, Category = "Branch")
    void ToggleAndTest();

    UFUNCTION(BlueprintCallable, Category = "Branch")
    void Test();

    UFUNCTION(BlueprintPure, Category = "Branch")
    bool GetValue() const { return bCurrentValue; }

    FSimpleActionDelegate& OnTrue() { return TrueEvent; }

    FSimpleActionDelegate& OnFalse() { return FalseEvent; }
};
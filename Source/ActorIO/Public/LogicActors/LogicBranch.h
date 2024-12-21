// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicBranch.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEvaluateBranch);

UCLASS()
class ACTORIO_API ALogicBranch : public ALogicActorBase
{
    GENERATED_BODY()

public:

    ALogicBranch();

public:

    virtual void RegisterIOEvents_Implementation(TArray<FActorIOEvent>& RegisteredEvents) override;
    virtual void RegisterIOFunctions_Implementation(TArray<FActorIOFunction>& RegisteredFunctions) override;

public:

    UPROPERTY(EditInstanceOnly, Category = "Branch")
    bool bInitialValue;

protected:

    bool bCurrentValue;

    FOnEvaluateBranch TrueEvent;

    FOnEvaluateBranch FalseEvent;

public:

    UFUNCTION(BlueprintCallable, Category = "Branch")
    void SetValue(bool bValue);

    UFUNCTION(BlueprintCallable, Category = "Branch")
    void SetValueAndEvaluate(bool bValue);

    UFUNCTION(BlueprintCallable, Category = "Branch")
    void Toggle();

    UFUNCTION(BlueprintCallable, Category = "Branch")
    void ToggleAndEvaluate();

    UFUNCTION(BlueprintCallable, Category = "Branch")
    void Evaluate();

    UFUNCTION(BlueprintPure, Category = "Branch")
    bool GetValue() const { return bCurrentValue; }

    FOnEvaluateBranch& OnTrue() { return TrueEvent; }

    FOnEvaluateBranch& OnFalse() { return FalseEvent; }
};
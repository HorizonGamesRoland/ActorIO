// Copyright 2025 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicCounter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCounterValue, int32, Value);

UCLASS()
class ACTORIO_API ALogicCounter : public ALogicActorBase
{
    GENERATED_BODY()

public:

    ALogicCounter();

public:

    UPROPERTY(EditInstanceOnly, Category = "Counter")
    int32 InitialValue;

    UPROPERTY(EditInstanceOnly, Category = "Counter")
    int32 TargetValue;

    UPROPERTY(EditInstanceOnly, Category = "Counter")
    bool bClampValue;

    UPROPERTY(BlueprintAssignable, Category = "Counter")
    FOnCounterValue OnValueChanged;

    UPROPERTY(BlueprintAssignable, Category = "Counter")
    FOnCounterValue OnTargetValueReached;

    UPROPERTY(BlueprintAssignable, Category = "Counter")
    FOnCounterValue OnGetValue;

protected:

    int32 CurrentValue;

public:

    virtual void PostInitializeComponents() override;
    virtual void RegisterIOEvents_Implementation(FActorIOEventList& RegisteredEvents) override;
    virtual void RegisterIOFunctions_Implementation(FActorIOFunctionList& RegisteredFunctions) override;

public:

    UFUNCTION(BlueprintCallable, Category = "Counter")
    void Add(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Counter")
    void Subtract(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Counter")
    void SetValue(int32 Value);

    UFUNCTION(BlueprintCallable, Category = "Counter")
    void SetTargetValue(int32 Value);

    UFUNCTION(BlueprintPure, Category = "Counter")
    int32 GetValue() const;

protected:

    UFUNCTION()
    void ProcessEvent_OnGetValue(int32 Value);
};
// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicCondition.generated.h"

UCLASS()
class ACTORIO_API ALogicCondition : public ALogicActorBase
{
    GENERATED_BODY()

public:

    ALogicCondition();

public:

    UPROPERTY(EditInstanceOnly, Category = "Condition")
    TObjectPtr<AActor> ObjectToTest;

    UPROPERTY(EditInstanceOnly, Category = "Condition")
    FName FunctionName;

    UPROPERTY(BlueprintAssignable, Category = "Condition")
    FSimpleActionDelegate OnPass;

    UPROPERTY(BlueprintAssignable, Category = "Condition")
    FSimpleActionDelegate OnFail;

public:

    virtual void RegisterIOEvents_Implementation(FActorIOEventList& RegisteredEvents) override;
    virtual void RegisterIOFunctions_Implementation(FActorIOFunctionList& RegisteredFunctions) override;

public:

    UFUNCTION(BlueprintCallable, Category = "Condition", meta = (AutoCreateRefTerm = "Arguments"))
    void Test(const FString& Arguments) const;

protected:

    bool PerformConditionCheck(FName InFunctionName, const FString& Arguments) const;
};
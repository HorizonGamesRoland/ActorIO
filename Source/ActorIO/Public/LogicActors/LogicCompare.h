// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicCompare.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCompareValue);

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

protected:

    FString CurrentValue;

    FOnCompareValue OnEquals;

    FOnCompareValue OnNotEquals;

    FOnCompareValue OnLessThen;

    FOnCompareValue OnGreaterThen;

public:

    virtual void PostInitializeComponents() override;
    virtual void RegisterIOEvents_Implementation(TArray<FActorIOEvent>& RegisteredEvents) override;
    virtual void RegisterIOFunctions_Implementation(TArray<FActorIOFunction>& RegisteredFunctions) override;

public:

    void SetValue(FString InValue);

    void SetValueAndCompare(FString InValue);

    void SetCompareValue(FString InValue);

    void Compare();

    FOnCompareValue& GetOnEquals() { return OnEquals; }

    FOnCompareValue& GetOnNotEquals() { return OnNotEquals; }

    FOnCompareValue& GetOnLessThen() { return OnLessThen; }

    FOnCompareValue& GetOnGreaterThen() { return OnGreaterThen; }
};
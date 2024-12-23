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

private:

    FString CurrentValue;

    FSimpleActionDelegate EqualsEvent;

    FSimpleActionDelegate NotEqualsEvent;

    FSimpleActionDelegate LessThenEvent;

    FSimpleActionDelegate GreaterThenEvent;

public:

    virtual void PostInitializeComponents() override;
    virtual void RegisterIOEvents_Implementation(TArray<FActorIOEvent>& RegisteredEvents) override;
    virtual void RegisterIOFunctions_Implementation(TArray<FActorIOFunction>& RegisteredFunctions) override;

public:

    void SetValue(FString InValue);

    void SetValueAndCompare(FString InValue);

    void SetCompareValue(FString InValue);

    void Compare();

    FSimpleActionDelegate& OnEquals() { return EqualsEvent; }

    FSimpleActionDelegate& OnNotEquals() { return NotEqualsEvent; }

    FSimpleActionDelegate& OnLessThen() { return LessThenEvent; }

    FSimpleActionDelegate& OnGreaterThen() { return GreaterThenEvent; }
};
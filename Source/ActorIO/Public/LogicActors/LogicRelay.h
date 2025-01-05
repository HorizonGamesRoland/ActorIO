// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicRelay.generated.h"

UCLASS()
class ACTORIO_API ALogicRelay : public ALogicActorBase
{
    GENERATED_BODY()

public:

    ALogicRelay();

public:

    UPROPERTY(EditInstanceOnly, Category = "Relay")
    bool bIsEnabled;

    UPROPERTY(EditInstanceOnly, Category = "Relay")
    bool bOnlyOnce;

    UPROPERTY(BlueprintAssignable, Category = "Relay")
    FSimpleActionDelegate OnTrigger;

protected:

    bool bWasTriggered;

public:

    virtual void RegisterIOEvents_Implementation(TArray<FActorIOEvent>& RegisteredEvents) override;
    virtual void RegisterIOFunctions_Implementation(TArray<FActorIOFunction>& RegisteredFunctions) override;

public:

    UFUNCTION(BlueprintCallable, Category = "Relay")
    void Trigger();

    UFUNCTION(BlueprintCallable, Category = "Relay")
    void Enable();

    UFUNCTION(BlueprintCallable, Category = "Relay")
    void Disable();

    UFUNCTION(BlueprintPure, Category = "Relay")
    bool IsEnabled() const;
};
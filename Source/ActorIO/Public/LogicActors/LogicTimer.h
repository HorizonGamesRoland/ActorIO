// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicTimer.generated.h"

UCLASS()
class ACTORIO_API ALogicTimer : public ALogicActorBase
{
    GENERATED_BODY()

public:

    ALogicTimer();

public:

    UPROPERTY(EditInstanceOnly, Category = "Timer")
    float Time;

    UPROPERTY(EditInstanceOnly, Category = "Timer")
    float TimeRandomization;

    UPROPERTY(EditInstanceOnly, Category = "Timer")
    bool bLoop;

    UPROPERTY(BlueprintAssignable, Category = "Timer")
    FSimpleActionDelegate OnTimer;

protected:

    FTimerHandle CurrentTimerHandle;

public:

    virtual void RegisterIOEvents_Implementation(TArray<FActorIOEvent>& RegisteredEvents) override;
    virtual void RegisterIOFunctions_Implementation(TArray<FActorIOFunction>& RegisteredFunctions) override;

public:

    UFUNCTION(BlueprintCallable, Category = "Timer")
    void StartTimer();

    UFUNCTION(BlueprintCallable, Category = "Timer")
    void StartTimerWithParams(float InTime, float InTimeRandomization, bool bInLoop);

    UFUNCTION(BlueprintCallable, Category = "Timer")
    void StopTimer();

    UFUNCTION(BlueprintPure, Category = "Timer")
    bool IsTimerActive() const;

protected:

    void OnTimerCallback();
};
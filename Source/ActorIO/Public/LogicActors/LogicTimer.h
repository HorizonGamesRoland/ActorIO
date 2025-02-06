// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicTimer.generated.h"

/**
 * An actor that fires an output at regular or random intervals.
 */
UCLASS()
class ACTORIO_API ALogicTimer : public ALogicActorBase
{
    GENERATED_BODY()

public:

    /** Default constructor. */
    ALogicTimer();

public:

    /** Base timer duration. 'TimerRandomization' is added to this. */
    UPROPERTY(EditInstanceOnly, Category = "Timer")
    float Time;

    /**
     * Min-max time for randomizing the base duration with.
     * For example if base time is 1.0 and randomization is 0.5, then result is between 0.5 - 1.5.
     */
    UPROPERTY(EditInstanceOnly, Category = "Timer")
    float TimeRandomization;

    /** Whether the timer should loop. */
    UPROPERTY(EditInstanceOnly, Category = "Timer")
    bool bLoop;

    /** Event when the timer expires. */
    UPROPERTY(BlueprintAssignable, Category = "Timer")
    FSimpleActionDelegate OnTimer;

protected:

    /** Handle for the current timer. */
    FTimerHandle CurrentTimerHandle;

protected:

    //~ Begin ALogicActorBase Interface
    virtual void RegisterIOEvents(FActorIOEventList& EventRegistry) override;
    virtual void RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry) override;
    //~ End ALogicActorBase Interface

public:

    /** Start the timer. If the timer is already active then it will be restarted. */
    UFUNCTION(BlueprintCallable, Category = "Timer")
    void StartTimer();

    /** Start the timer with custom params. If the timer is already active then it will be restarted. */
    UFUNCTION(BlueprintCallable, Category = "Timer")
    void StartTimerWithParams(float InTime, float InTimeRandomization, bool bInLoop);

    /** Stop the timer if it is active. */
    UFUNCTION(BlueprintCallable, Category = "Timer")
    void StopTimer();

    /** Get whether the timer is active or not. */
    UFUNCTION(BlueprintPure, Category = "Timer")
    bool IsTimerActive() const;

protected:

    /** Called when the timer is finished with a cycle. */
    void OnTimerCallback();
};
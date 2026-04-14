// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicTimer.generated.h"

/**
 * Properties of the timer, representing the overall logical state.
 */
USTRUCT()
struct ACTORIO_API FTimerLogicProperties
{
    GENERATED_BODY()

    /** The actual rate of the timer. */
    UPROPERTY(SaveGame)
    float TimerRate;

    /** Whether the timer is looping or not. The actual timer may not be looping (see 'bRandomizeTimeEachLoop'). */
    UPROPERTY(SaveGame)
    bool bLoop;

    /** Time remaining on the timer. Only updated before serialization! */
    UPROPERTY(SaveGame)
    float RemainingTime;

    /** Custom base rate value of the timer. -1 if no override. */
    UPROPERTY(SaveGame)
    float BaseRateOverride;

    /** Custom time randomization. -1 if no override. */
    UPROPERTY(SaveGame)
    float RateRandomizationOverride;

    /** Default constructor. */
    FTimerLogicProperties() :
        TimerRate(0.0f),
        bLoop(false),
        RemainingTime(-1.0f),
        BaseRateOverride(-1.0f),
        RateRandomizationOverride(-1.0f)
    {}
};

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

    /**
     * Base timer duration. 'TimeRandomization' will be added to this.
     * If 0.0, then the timer will delay for one frame.
     */
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

    /**
     * Whether to apply time randomization before each loop, or just once at the start.
     * This causes the timer to not actually loop, but rather restart for each cycle.
     * Not recommended for very fast ticking timers, where the timer rate may exceed the frame time.
     */
    UPROPERTY(EditInstanceOnly, Category = "Timer", meta = (EditCondition = "bLoop"))
    bool bRandomizeTimeEachLoop;

    /** Whether to start the timer automatically during 'BeginPlay'. */
    UPROPERTY(EditInstanceOnly, Category = "Timer")
    bool bAutoStart;

    /** Event when the timer expires. */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FSimpleActionDelegate OnTimer;

protected:

    /** Handle for the current timer. */
    FTimerHandle CurrentTimerHandle;

    /** Properties of the current/last timer. Only updated at specific times. */
    UPROPERTY(SaveGame)
    FTimerLogicProperties TimerProperties;

public:

    //~ Begin ALogicActorBase Interface
    virtual void RegisterIOEvents(FActorIOEventList& EventRegistry) override;
    virtual void RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry) override;
    virtual void BeginPlay() override;
    virtual void PreSerializeLogicActor(FArchive& Ar) override;
    virtual void PostSerializeLogicActor(FArchive& Ar) override;
    //~ End ALogicActorBase Interface

public:

    /** Start the timer. If the timer is already active then it will be restarted. */
    UFUNCTION(BlueprintCallable, Category = "LogicActors|LogicTimer")
    void StartTimer();

    /** Start the timer with custom params. If the timer is already active then it will be restarted. */
    UFUNCTION(BlueprintCallable, Category = "LogicActors|LogicTimer")
    void StartTimerWithParams(float InTime, float InTimeRandomization, bool bInLoop);

    /** Stop the timer if it is active. */
    UFUNCTION(BlueprintCallable, Category = "LogicActors|LogicTimer")
    void StopTimer();

    /** Get whether the timer is active or not. */
    UFUNCTION(BlueprintPure, Category = "LogicActors|LogicTimer")
    bool IsTimerActive() const;

protected:

    /** Start a new timer. Stops previous timer if needed. */
    void SetTimer(float InTime, bool bInLoop);

    /** Called when the timer is finished with a cycle. */
    void OnTimerCallback();
};
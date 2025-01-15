// Copyright 2025 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveFloat.h"
#include "LogicTimeline.generated.h"

/** Delegate with the timeline's current value. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimelineValueChanged, float, Value);

/**
 * An actor that runs a timeline evaluating the value of a given curve.
 * Use this to animate things, or to interpolate values.
 */
UCLASS()
class ACTORIO_API ALogicTimeline : public ALogicActorBase
{
    GENERATED_BODY()

public:

    /** Default constructor. */
    ALogicTimeline();

public:

    /** Curve to evaluate. */
    UPROPERTY(EditInstanceOnly, Category = "Timeline")
    FRuntimeFloatCurve Curve;

    /** How fast to play through the curve. */
    UPROPERTY(EditInstanceOnly, Category = "Timeline")
    float PlayRate;

    /** Whether the timeline should loop when it reaches the end, or stop. */
    UPROPERTY(EditInstanceOnly, Category = "Timeline")
    bool bLoop;

    /** Whether the timeline is affected by time dilation (e.g. slow motion). */
    UPROPERTY(EditInstanceOnly, Category = "Timeline")
    bool bIgnoreTimeDilation;

    /** Event when the timeline's value is changed. */
    UPROPERTY(BlueprintAssignable, Category = "Timeline")
    FOnTimelineValueChanged OnTimelineValueChanged;

    /** Event when the timeline is finished. */
    UPROPERTY(BlueprintAssignable, Category = "Timeline")
    FSimpleActionDelegate OnTimelineFinished;

protected:

    /** The actual timeline that is responsible for evaluating the curve. */
    FTimeline Timeline;

    /** The curve asset that the timeline is evaluating. */
    UPROPERTY(Transient)
    TObjectPtr<UCurveFloat> TimelineCurve;

public:

    //~ Begin ALogicActorBase Interface
    virtual void PostInitializeComponents() override;
    virtual void RegisterIOEvents_Implementation(FActorIOEventList& RegisteredEvents) override;
    virtual void RegisterIOFunctions_Implementation(FActorIOFunctionList& RegisteredFunctions) override;
    virtual void Tick(float DeltaSeconds) override;
    //~ End ALogicActorBase Interface

public:

    /** Start the timeline. */
    UFUNCTION(BlueprintCallable, Category = "Timeline")
    void Play();

    /** Start the timeline from the start. */
    UFUNCTION(BlueprintCallable, Category = "Timeline")
    void PlayFromStart();

    /** Reverse the timeline. */
    UFUNCTION(BlueprintCallable, Category = "Timeline")
    void Reverse();

    /** Reverse the timeline from the end. */
    UFUNCTION(BlueprintCallable, Category = "Timeline")
    void ReverseFromEnd();

    /** Stop the timeline. */
    UFUNCTION(BlueprintCallable, Category = "Timeline")
    void Stop();

protected:

    /** Called when the timeline's value is changed. */
    void OnTimelineValueChangedCallback(float Output);

    /** Called when the timeline is finished. */
    void OnTimelineFinishedCallback();

protected:

    /** Event processor for the 'OnTimelineValueChanged' event. */
    UFUNCTION()
    void ProcessEvent_OnTimelineValueChanged(float InValue);
};
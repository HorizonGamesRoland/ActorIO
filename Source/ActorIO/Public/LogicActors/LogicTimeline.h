// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveFloat.h"
#include "LogicTimeline.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimelineValueChanged, float, Value);

UCLASS()
class ACTORIO_API ALogicTimeline : public ALogicActorBase
{
    GENERATED_BODY()

public:

    ALogicTimeline();

public:

    UPROPERTY(EditInstanceOnly, Category = "Timeline")
    FRuntimeFloatCurve Curve;

    UPROPERTY(EditInstanceOnly, Category = "Timeline")
    float PlayRate;

    UPROPERTY(EditInstanceOnly, Category = "Timeline")
    bool bLoop;

    UPROPERTY(EditInstanceOnly, Category = "Timeline")
    bool bIgnoreTimeDilation;

    UPROPERTY(BlueprintAssignable, Category = "Timeline")
    FOnTimelineValueChanged OnTimelineValueChanged;

    UPROPERTY(BlueprintAssignable, Category = "Timeline")
    FSimpleActionDelegate OnTimelineFinished;

protected:

    FTimeline Timeline;

    UPROPERTY(Transient)
    TObjectPtr<UCurveFloat> TimelineCurve;

public:

    virtual void PostInitializeComponents() override;
    virtual void RegisterIOEvents_Implementation(FActorIOEventList& RegisteredEvents) override;
    virtual void RegisterIOFunctions_Implementation(FActorIOFunctionList& RegisteredFunctions) override;
    virtual void Tick(float DeltaSeconds) override;

public:

    UFUNCTION(BlueprintCallable, Category = "Timeline")
    void Play();

    UFUNCTION(BlueprintCallable, Category = "Timeline")
    void PlayFromStart();

    UFUNCTION(BlueprintCallable, Category = "Timeline")
    void Reverse();

    UFUNCTION(BlueprintCallable, Category = "Timeline")
    void ReverseFromEnd();

    UFUNCTION(BlueprintCallable, Category = "Timeline")
    void Stop();

protected:

    void OnTimelineValueChangedCallback(float Output);

    void OnTimelineFinishedCallback();
};
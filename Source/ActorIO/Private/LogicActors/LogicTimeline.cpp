// Copyright 2024 Horizon Games. All Rights Reserved.

#include "LogicActors/LogicTimeline.h"

#define LOCTEXT_NAMESPACE "ActorIO"

ALogicTimeline::ALogicTimeline()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	Curve = FRuntimeFloatCurve();
	PlayRate = 1.0f;
	bLoop = false;
	bIgnoreTimeDilation = false;

	Timeline = FTimeline();
}

void ALogicTimeline::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UWorld* MyWorld = GetWorld();
	if (!MyWorld || !MyWorld->IsGameWorld())
	{
		// Do nothing in the editor.
		return;
	}

	if (Curve.ExternalCurve)
	{
		TimelineCurve = Curve.ExternalCurve;
	}
	else
	{
		UCurveFloat* NewCurve = NewObject<UCurveFloat>(this, NAME_None, RF_Transient);
		NewCurve->FloatCurve = *Curve.GetRichCurve();
		TimelineCurve = NewCurve;
	}

	Timeline.AddInterpFloat(TimelineCurve, FOnTimelineFloatStatic::CreateUObject(this, &ThisClass::OnTimelineFloatCallback));
	Timeline.SetTimelineFinishedFunc(FOnTimelineEventStatic::CreateUObject(this, &ThisClass::OnTimelineFinishedCallback));
}

void ALogicTimeline::RegisterIOEvents_Implementation(TArray<FActorIOEvent>& RegisteredEvents)
{
	RegisteredEvents.Add(FActorIOEvent()
		.SetId(TEXT("ALogicTimeline::OnTimelineFloat"))
		.SetDisplayName(LOCTEXT("ALogicTimeline.OnTimelineFloat", "OnTimelineFloat"))
		.SetTooltipText(LOCTEXT("ALogicTimeline.OnTimelineFloatTooltip", "Event when the timeline's value changes."))
		.SetMulticastDelegate(this, &OnTimelineValueChanged));

	RegisteredEvents.Add(FActorIOEvent()
		.SetId(TEXT("ALogicTimeline::OnTimelineFinished"))
		.SetDisplayName(LOCTEXT("ALogicTimeline.OnTimelineFinished", "OnTimelineFinished"))
		.SetTooltipText(LOCTEXT("ALogicTimeline.OnTimelineFinishedTooltip", "Event when the timeline is finished."))
		.SetMulticastDelegate(this, &OnTimelineFinished));
}

void ALogicTimeline::RegisterIOFunctions_Implementation(TArray<FActorIOFunction>& RegisteredFunctions)
{
	RegisteredFunctions.Add(FActorIOFunction()
		.SetId(TEXT("ALogicTimeline::Play"))
		.SetDisplayName(LOCTEXT("ALogicTimeline.Play", "Play"))
		.SetTooltipText(LOCTEXT("ALogicTimeline.PlayTooltip", "Start the timeline."))
		.SetFunction(TEXT("Play")));

	RegisteredFunctions.Add(FActorIOFunction()
		.SetId(TEXT("ALogicTimeline::PlayFromStart"))
		.SetDisplayName(LOCTEXT("ALogicTimeline.PlayFromStart", "PlayFromStart"))
		.SetTooltipText(LOCTEXT("ALogicTimeline.PlayFromStartTooltip", "Start the timeline from the start."))
		.SetFunction(TEXT("PlayFromStart")));

	RegisteredFunctions.Add(FActorIOFunction()
		.SetId(TEXT("ALogicTimeline::Reverse"))
		.SetDisplayName(LOCTEXT("ALogicTimeline.Reverse", "Reverse"))
		.SetTooltipText(LOCTEXT("ALogicTimeline.ReverseTooltip", "Reverse the timeline."))
		.SetFunction(TEXT("Reverse")));

	RegisteredFunctions.Add(FActorIOFunction()
		.SetId(TEXT("ALogicTimeline::ReverseFromEnd"))
		.SetDisplayName(LOCTEXT("ALogicTimeline.ReverseFromEnd", "ReverseFromEnd"))
		.SetTooltipText(LOCTEXT("ALogicTimeline.ReverseFromEndTooltip", "Reverse the timeline from the end."))
		.SetFunction(TEXT("ReverseFromEnd")));

	RegisteredFunctions.Add(FActorIOFunction()
		.SetId(TEXT("ALogicTimeline::Stop"))
		.SetDisplayName(LOCTEXT("ALogicTimeline.Stop", "Stop"))
		.SetTooltipText(LOCTEXT("ALogicTimeline.StopTooltip", "Stop the timeline."))
		.SetFunction(TEXT("Stop")));
}

void ALogicTimeline::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// #NOTE: Code here is mostly same as UTimelineComponent::TickComponent.

	if (bIgnoreTimeDilation)
	{
		// Get the raw, undilated delta time.
		DeltaSeconds = FApp::GetDeltaTime();
		const UWorld* World = GetWorld();
		if (const AWorldSettings* WorldSettings = World ? World->GetWorldSettings() : nullptr)
		{
			// Clamp delta time in the same way as before.
			// UWorld::Tick called AWorldSettings::FixupDeltaSeconds, which clamped between Min and MaxUndilatedFrameTime.
			DeltaSeconds = FMath::Clamp(DeltaSeconds, WorldSettings->MinUndilatedFrameTime, WorldSettings->MaxUndilatedFrameTime);
		}
	}

	Timeline.TickTimeline(DeltaSeconds);

	if (!Timeline.IsPlaying())
	{
		SetActorTickEnabled(false);
	}
}

void ALogicTimeline::Play()
{
	SetActorTickEnabled(true);
	Timeline.Play();
}

void ALogicTimeline::PlayFromStart()
{
	SetActorTickEnabled(true);
	Timeline.PlayFromStart();
}

void ALogicTimeline::Reverse()
{
	SetActorTickEnabled(true);
	Timeline.Reverse();
}

void ALogicTimeline::ReverseFromEnd()
{
	SetActorTickEnabled(true);
	Timeline.ReverseFromEnd();
}

void ALogicTimeline::Stop()
{
	Timeline.Stop();
}

void ALogicTimeline::OnTimelineFloatCallback(float Output)
{
	OnTimelineValueChanged.Broadcast(Output);
}

void ALogicTimeline::OnTimelineFinishedCallback()
{
	OnTimelineFinished.Broadcast();
}

#undef LOCTEXT_NAMESPACE
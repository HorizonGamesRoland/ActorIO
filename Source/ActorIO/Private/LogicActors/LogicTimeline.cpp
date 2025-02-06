// Copyright 2025 Horizon Games. All Rights Reserved.

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

#if WITH_EDITORONLY_DATA
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture(TEXT("/ActorIO/AssetIcons/S_Timeline"));
	if (SpriteComponent && SpriteTexture.Succeeded())
	{
		SpriteComponent->SetSprite(SpriteTexture.Get());
		SpriteComponent->SetRelativeScale3D_Direct(FVector(1.0f));
	}
#endif
}

void ALogicTimeline::RegisterIOEvents(FActorIOEventList& EventRegistry)
{
	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicTimeline::OnValueChanged"))
		.SetDisplayName(LOCTEXT("ALogicTimeline.OnValueChanged", "OnValueChanged"))
		.SetTooltipText(LOCTEXT("ALogicTimeline.OnValueChangedTooltip", "Event when the timeline's value is changed."))
		.SetMulticastDelegate(this, &OnTimelineValueChanged)
		.SetEventProcessor(this, TEXT("ProcessEvent_OnTimelineValueChanged")));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicTimeline::OnFinished"))
		.SetDisplayName(LOCTEXT("ALogicTimeline.OnFinished", "OnFinished"))
		.SetTooltipText(LOCTEXT("ALogicTimeline.OnFinishedTooltip", "Event when the timeline is finished."))
		.SetMulticastDelegate(this, &OnTimelineFinished));
}

void ALogicTimeline::RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry)
{
	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicTimeline::Play"))
		.SetDisplayName(LOCTEXT("ALogicTimeline.Play", "Play"))
		.SetTooltipText(LOCTEXT("ALogicTimeline.PlayTooltip", "Start the timeline."))
		.SetFunction(TEXT("Play")));

	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicTimeline::PlayFromStart"))
		.SetDisplayName(LOCTEXT("ALogicTimeline.PlayFromStart", "PlayFromStart"))
		.SetTooltipText(LOCTEXT("ALogicTimeline.PlayFromStartTooltip", "Start the timeline from the start."))
		.SetFunction(TEXT("PlayFromStart")));

	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicTimeline::Reverse"))
		.SetDisplayName(LOCTEXT("ALogicTimeline.Reverse", "Reverse"))
		.SetTooltipText(LOCTEXT("ALogicTimeline.ReverseTooltip", "Reverse the timeline."))
		.SetFunction(TEXT("Reverse")));

	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicTimeline::ReverseFromEnd"))
		.SetDisplayName(LOCTEXT("ALogicTimeline.ReverseFromEnd", "ReverseFromEnd"))
		.SetTooltipText(LOCTEXT("ALogicTimeline.ReverseFromEndTooltip", "Reverse the timeline from the end."))
		.SetFunction(TEXT("ReverseFromEnd")));

	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicTimeline::Stop"))
		.SetDisplayName(LOCTEXT("ALogicTimeline.Stop", "Stop"))
		.SetTooltipText(LOCTEXT("ALogicTimeline.StopTooltip", "Stop the timeline."))
		.SetFunction(TEXT("Stop")));
}

void ALogicTimeline::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UWorld* MyWorld = GetWorld();
	if (MyWorld && MyWorld->IsGameWorld())
	{
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

		Timeline.AddInterpFloat(TimelineCurve, FOnTimelineFloatStatic::CreateUObject(this, &ThisClass::OnTimelineValueChangedCallback));
		Timeline.SetTimelineFinishedFunc(FOnTimelineEventStatic::CreateUObject(this, &ThisClass::OnTimelineFinishedCallback));
	}
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

void ALogicTimeline::OnTimelineValueChangedCallback(float Output)
{
	OnTimelineValueChanged.Broadcast(Output);
}

void ALogicTimeline::OnTimelineFinishedCallback()
{
	OnTimelineFinished.Broadcast();
}

void ALogicTimeline::ProcessEvent_OnTimelineValueChanged(float InValue)
{
	FActionExecutionContext& ExecContext = FActionExecutionContext::Get(this);
	ExecContext.SetNamedArgument(TEXT("$Value"), FString::SanitizeFloat(InValue));
}

#undef LOCTEXT_NAMESPACE
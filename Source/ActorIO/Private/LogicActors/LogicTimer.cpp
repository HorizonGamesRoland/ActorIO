// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "LogicActors/LogicTimer.h"
#include "Engine/World.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "ActorIO"

ALogicTimer::ALogicTimer()
{
	Time = 0.0f;
	TimeRandomization = 0.0f;
	bLoop = false;
	bRandomizeTimeEachLoop = false;
	bAutoStart = false;

	CurrentTimerHandle = FTimerHandle();
	TimerProperties = FTimerLogicProperties();

#if WITH_EDITORONLY_DATA
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture(TEXT("/ActorIO/AssetIcons/S_Timer"));
	if (SpriteComponent && SpriteTexture.Succeeded())
	{
		SpriteComponent->SetSprite(SpriteTexture.Get());
		SpriteComponent->SetRelativeScale3D_Direct(FVector(1.0f));
	}
#endif
}

void ALogicTimer::RegisterIOEvents(FActorIOEventList& EventRegistry)
{
	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicTimer::OnTimer"))
		.SetDisplayName(LOCTEXT("ALogicTimer.OnTimer", "OnTimer"))
		.SetTooltipText(LOCTEXT("ALogicTimer.OnTimerTooltip", "Event when the timer expires."))
		.SetMulticastDelegate(this, &OnTimer));
}

void ALogicTimer::RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry)
{
	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicTimer::StartTimer"))
		.SetDisplayName(LOCTEXT("ALogicTimer.StartTimer", "StartTimer"))
		.SetTooltipText(LOCTEXT("ALogicTimer.StartTimerTooltip", "Start the timer. If the timer is already active then it will be restarted."))
		.SetFunction(TEXT("StartTimer")));

	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicTimer::StartTimerWithParams"))
		.SetDisplayName(LOCTEXT("ALogicTimer.StartTimerWithParams", "StartTimerWithParams"))
		.SetTooltipText(LOCTEXT("ALogicTimer.StartTimerWithParamsTooltip", "Start the timer with custom params. If the timer is already active then it will be restarted."))
		.SetFunction(TEXT("StartTimerWithParams")));

	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicTimer::StopTimer"))
		.SetDisplayName(LOCTEXT("ALogicTimer.StopTimer", "StopTimer"))
		.SetTooltipText(LOCTEXT("ALogicTimer.StopTimerTooltip", "Stop the timer if it is active."))
		.SetFunction(TEXT("StopTimer")));
}

void ALogicTimer::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoStart)
	{
		StartTimer();
	}
}

void ALogicTimer::PreSerializeLogicActor(FArchive& Ar)
{
	if (Ar.IsSaving() && Ar.IsSaveGame())
	{
		FTimerManager& TimerManager = GetWorld()->GetTimerManager();
		TimerProperties.RemainingTime = TimerManager.GetTimerRemaining(CurrentTimerHandle);
	}
}

void ALogicTimer::PostSerializeLogicActor(FArchive& Ar)
{
	if (Ar.IsLoading() && Ar.IsSaveGame())
	{
		FTimerManager& TimerManager = GetWorld()->GetTimerManager();
		if (TimerProperties.RemainingTime != -1.0f)
		{
			SetTimer(TimerProperties.RemainingTime, false);
		}
	}
}

void ALogicTimer::StartTimer()
{
	StartTimerWithParams(Time, TimeRandomization, bLoop);
}

void ALogicTimer::StartTimerWithParams(float InTime, float InTimeRandomization, bool bInLoop)
{
	TimerProperties.TimerRate = InTime + FMath::RandRange(-InTimeRandomization, InTimeRandomization);
	TimerProperties.bLoop = bInLoop;
	TimerProperties.RemainingTime = -1.0f;
	TimerProperties.BaseRateOverride = FMath::IsNearlyEqual(InTime, Time) ? -1.0f : InTime;
	TimerProperties.RateRandomizationOverride = FMath::IsNearlyEqual(InTimeRandomization, TimeRandomization) ? -1.0f : InTimeRandomization;

	SetTimer(TimerProperties.TimerRate, TimerProperties.bLoop);
}

void ALogicTimer::StopTimer()
{
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.ClearTimer(CurrentTimerHandle);
}

bool ALogicTimer::IsTimerActive() const
{
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	return TimerManager.IsTimerActive(CurrentTimerHandle);
}

void ALogicTimer::SetTimer(float InTime, bool bInLoop)
{
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if (TimerManager.IsTimerActive(CurrentTimerHandle))
	{
		TimerManager.ClearTimer(CurrentTimerHandle);
	}

	const FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ThisClass::OnTimerCallback);
	if (InTime > 0.0f)
	{
		TimerManager.SetTimer(CurrentTimerHandle, TimerDelegate, InTime, bInLoop);
	}
	else
	{
		CurrentTimerHandle = TimerManager.SetTimerForNextTick(TimerDelegate);
	}
}

void ALogicTimer::OnTimerCallback()
{
	OnTimer.Broadcast();
	
	const bool bWasCatchupTimer = TimerProperties.RemainingTime >= 0.0f;
	TimerProperties.RemainingTime = -1.0f;

	if (TimerProperties.bLoop && (bWasCatchupTimer || bRandomizeTimeEachLoop))
	{
		if (bRandomizeTimeEachLoop)
		{
			const float BaseRate = TimerProperties.BaseRateOverride != -1.0f ? TimerProperties.BaseRateOverride : Time;
			const float RateRandomization = TimerProperties.RateRandomizationOverride != -1.0f ? TimerProperties.RateRandomizationOverride : TimeRandomization;

			TimerProperties.TimerRate = BaseRate + FMath::RandRange(-RateRandomization, RateRandomization);

			// Not actually looping because we want to randomize timer rate each loop.
			SetTimer(TimerProperties.TimerRate, false);
		}
		else
		{
			SetTimer(TimerProperties.TimerRate, true);
		}
	}
}

#undef LOCTEXT_NAMESPACE
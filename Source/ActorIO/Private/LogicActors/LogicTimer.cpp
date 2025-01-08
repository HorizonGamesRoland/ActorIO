// Copyright 2024 Horizon Games. All Rights Reserved.

#include "LogicActors/LogicTimer.h"

#define LOCTEXT_NAMESPACE "ActorIO"

ALogicTimer::ALogicTimer()
{
	CurrentTimerHandle = FTimerHandle();
}

void ALogicTimer::RegisterIOEvents_Implementation(FActorIOEventList& RegisteredEvents)
{
	RegisteredEvents.Add(FActorIOEvent()
		.SetId(TEXT("ALogicTimer::OnTimer"))
		.SetDisplayName(LOCTEXT("ALogicTimer.OnTimer", "OnTimer"))
		.SetTooltipText(LOCTEXT("ALogicTimer.OnTimerTooltip", "Event when the timer expires."))
		.SetMulticastDelegate(this, &OnTimer));
}

void ALogicTimer::RegisterIOFunctions_Implementation(FActorIOFunctionList& RegisteredFunctions)
{
	RegisteredFunctions.Add(FActorIOFunction()
		.SetId(TEXT("ALogicTimer::StartTimer"))
		.SetDisplayName(LOCTEXT("ALogicTimer.StartTimer", "StartTimer"))
		.SetTooltipText(LOCTEXT("ALogicTimer.StartTimerTooltip", "Start the timer. If the timer is already active then it will be restarted."))
		.SetFunction(TEXT("StartTimer")));

	RegisteredFunctions.Add(FActorIOFunction()
		.SetId(TEXT("ALogicTimer::StartTimerWithParams"))
		.SetDisplayName(LOCTEXT("ALogicTimer.StartTimerWithParams", "StartTimerWithParams"))
		.SetTooltipText(LOCTEXT("ALogicTimer.StartTimerWithParamsTooltip", "Start the timer with custom params. If the timer is already active then it will be restarted."))
		.SetFunction(TEXT("StartTimerWithParams")));

	RegisteredFunctions.Add(FActorIOFunction()
		.SetId(TEXT("ALogicTimer::StopTimer"))
		.SetDisplayName(LOCTEXT("ALogicTimer.StopTimer", "StopTimer"))
		.SetTooltipText(LOCTEXT("ALogicTimer.StopTimerTooltip", "Stop the timer if it is active."))
		.SetFunction(TEXT("StopTimer")));
}

void ALogicTimer::StartTimer()
{
	const float TimerRate = Time + FMath::RandRange(-TimeRandomization, TimeRandomization);

	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ThisClass::OnTimerCallback);
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.SetTimer(CurrentTimerHandle, TimerDelegate, TimerRate, bLoop);
}

void ALogicTimer::StartTimerWithParams(float InTime, float InTimeRandomization, bool bInLoop)
{
	const float TimerRate = InTime + FMath::RandRange(-InTimeRandomization, InTimeRandomization);

	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ThisClass::OnTimerCallback);
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.SetTimer(CurrentTimerHandle, TimerDelegate, TimerRate, bInLoop);
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

void ALogicTimer::OnTimerCallback()
{
	OnTimer.Broadcast();
}

#undef LOCTEXT_NAMESPACE
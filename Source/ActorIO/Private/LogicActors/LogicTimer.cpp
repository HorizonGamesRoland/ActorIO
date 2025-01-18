// Copyright 2025 Horizon Games. All Rights Reserved.

#include "LogicActors/LogicTimer.h"

#define LOCTEXT_NAMESPACE "ActorIO"

ALogicTimer::ALogicTimer()
{
	CurrentTimerHandle = FTimerHandle();

#if WITH_EDITORONLY_DATA
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture(TEXT("/ActorIO/S_Timer"));
	if (SpriteComponent && SpriteTexture.Succeeded())
	{
		SpriteComponent->SetSprite(SpriteTexture.Get());
		SpriteComponent->SetRelativeScale3D_Direct(FVector(1.0f));
	}
#endif
}

void ALogicTimer::RegisterIOEvents_Implementation(FActorIOEventList& EventRegistry)
{
	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicTimer::OnTimer"))
		.SetDisplayName(LOCTEXT("ALogicTimer.OnTimer", "OnTimer"))
		.SetTooltipText(LOCTEXT("ALogicTimer.OnTimerTooltip", "Event when the timer expires."))
		.SetMulticastDelegate(this, &OnTimer));
}

void ALogicTimer::RegisterIOFunctions_Implementation(FActorIOFunctionList& FunctionRegistry)
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

void ALogicTimer::StartTimer()
{
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ThisClass::OnTimerCallback);
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();

	const float TimerRate = Time + FMath::RandRange(-TimeRandomization, TimeRandomization);
	if (TimerRate > 0.0f)
	{
		TimerManager.SetTimer(CurrentTimerHandle, TimerDelegate, TimerRate, bLoop);
	}
	else
	{
		CurrentTimerHandle = TimerManager.SetTimerForNextTick(TimerDelegate);
	}
}

void ALogicTimer::StartTimerWithParams(float InTime, float InTimeRandomization, bool bInLoop)
{
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ThisClass::OnTimerCallback);
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	
	const float TimerRate = InTime + FMath::RandRange(-InTimeRandomization, InTimeRandomization);
	if (TimerRate > 0.0f)
	{
		TimerManager.SetTimer(CurrentTimerHandle, TimerDelegate, TimerRate, bLoop);
	}
	else
	{
		CurrentTimerHandle = TimerManager.SetTimerForNextTick(TimerDelegate);
	}
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
// Copyright 2024 Horizon Games. All Rights Reserved.

#include "LogicActors/LogicRelay.h"

#define LOCTEXT_NAMESPACE "ActorIO"

ALogicRelay::ALogicRelay()
{
	bIsEnabled = true;
	bOnlyOnce = false;
	bWasTriggered = false;
}

void ALogicRelay::RegisterIOEvents_Implementation(TArray<FActorIOEvent>& RegisteredEvents)
{
	RegisteredEvents.Add(FActorIOEvent()
		.SetId(TEXT("ALogicRelay::OnTrigger"))
		.SetDisplayName(LOCTEXT("ALogicRelay.OnTrigger", "OnTrigger"))
		.SetTooltipText(LOCTEXT("ALogicRelay.OnTriggerTooltip", "Event when the relay is triggered."))
		.SetMulticastDelegate(this, &TriggerEvent));
}

void ALogicRelay::RegisterIOFunctions_Implementation(TArray<FActorIOFunction>& RegisteredFunctions)
{
	RegisteredFunctions.Add(FActorIOFunction()
		.SetId(TEXT("ALogicRelay::Trigger"))
		.SetDisplayName(LOCTEXT("ALogicRelay.Trigger", "Trigger"))
		.SetTooltipText(LOCTEXT("ALogicRelay.TriggerTooltip", "Trigger the relay, causing the 'OnTrigger' event to fire if it is enabled."))
		.SetFunction(TEXT("Trigger")));

	RegisteredFunctions.Add(FActorIOFunction()
		.SetId(TEXT("ALogicRelay::Enable"))
		.SetDisplayName(LOCTEXT("ALogicRelay.Enable", "Enable"))
		.SetTooltipText(LOCTEXT("ALogicRelay.EnableTooltip", "Allow the relay to fire the 'OnTrigger' event."))
		.SetFunction(TEXT("Enable")));

	RegisteredFunctions.Add(FActorIOFunction()
		.SetId(TEXT("ALogicRelay::Disable"))
		.SetDisplayName(LOCTEXT("ALogicRelay.Disable", "Disable"))
		.SetTooltipText(LOCTEXT("ALogicRelay.DisableTooltip", "Prevent the relay from firing the 'OnTrigger' event."))
		.SetFunction(TEXT("Disable")));
}

void ALogicRelay::Trigger()
{
	if (bIsEnabled)
	{
		if (bOnlyOnce && bWasTriggered)
		{
			// Do nothing if the relay was triggered once already and we only want it once.
			return;
		}

		TriggerEvent.Broadcast();
		bWasTriggered = true;
	}
}

void ALogicRelay::Enable()
{
	bIsEnabled = true;
}

void ALogicRelay::Disable()
{
	bIsEnabled = false;
}

bool ALogicRelay::IsEnabled() const
{
	return bIsEnabled;
}

#undef LOCTEXT_NAMESPACE
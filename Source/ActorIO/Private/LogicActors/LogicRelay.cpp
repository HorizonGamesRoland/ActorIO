// Copyright 2025 Horizon Games. All Rights Reserved.

#include "LogicActors/LogicRelay.h"

#define LOCTEXT_NAMESPACE "ActorIO"

ALogicRelay::ALogicRelay()
{
	bIsEnabled = true;

#if WITH_EDITORONLY_DATA
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture(TEXT("/ActorIO/S_Relay"));
	if (SpriteComponent && SpriteTexture.Succeeded())
	{
		SpriteComponent->SetSprite(SpriteTexture.Get());
		SpriteComponent->SetRelativeScale3D_Direct(FVector(1.0f));
	}
#endif
}

void ALogicRelay::RegisterIOEvents_Implementation(FActorIOEventList& RegisteredEvents)
{
	RegisteredEvents.Add(FActorIOEvent()
		.SetId(TEXT("ALogicRelay::OnTrigger"))
		.SetDisplayName(LOCTEXT("ALogicRelay.OnTrigger", "OnTrigger"))
		.SetTooltipText(LOCTEXT("ALogicRelay.OnTriggerTooltip", "Event when the relay is triggered."))
		.SetMulticastDelegate(this, &OnTrigger));
}

void ALogicRelay::RegisterIOFunctions_Implementation(FActorIOFunctionList& RegisteredFunctions)
{
	RegisteredFunctions.Add(FActorIOFunction()
		.SetId(TEXT("ALogicRelay::Trigger"))
		.SetDisplayName(LOCTEXT("ALogicRelay.Trigger", "Trigger"))
		.SetTooltipText(LOCTEXT("ALogicRelay.TriggerTooltip", "Trigger the relay, causing the 'OnTrigger' event to fire if enabled."))
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
		OnTrigger.Broadcast();
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
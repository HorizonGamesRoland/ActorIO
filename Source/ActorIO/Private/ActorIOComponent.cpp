// Copyright 2024 Horizon Games. All Rights Reserved.

#include "ActorIOComponent.h"
#include "ActorIOAction.h"
#include "ActorIOInterface.h"
#include "GameFramework/Actor.h"
#include "Particles/Emitter.h"

UActorIOComponent::UActorIOComponent()
{
	Actions = TArray<TObjectPtr<UActorIOAction>>();
}

void UActorIOComponent::OnRegister()
{
	Super::OnRegister();

	// #TODO: Ensure action list does not have nullptr

	UWorld* MyWorld = GetWorld();
	if (!MyWorld || !MyWorld->IsGameWorld())
	{
		// Do nothing in the editor (e.g level editor, blueprint).
		return;
	}

	BindActions();
}

void UActorIOComponent::BindActions()
{
	for (int32 ActionIdx = 0; ActionIdx != Actions.Num(); ++ActionIdx)
	{
		UActorIOAction* Action = Actions[ActionIdx].Get();
		if (Action)
		{
			Action->BindAction();
		}
	}
}

void UActorIOComponent::UnbindActions()
{
	for (int32 ActionIdx = 0; ActionIdx != Actions.Num(); ++ActionIdx)
	{
		UActorIOAction* Action = Actions[ActionIdx].Get();
		if (Action)
		{
			Action->UnbindAction();
		}
	}
}

TArray<FActorIOEvent> UActorIOComponent::GetEventsForObject(AActor* InObject)
{
	TArray<FActorIOEvent> OutEvents = UActorIOComponent::GetNativeEventsForObject(InObject);
	if (InObject && InObject->Implements<UActorIOInterface>())
	{
		IActorIOInterface::Execute_RegisterIOEvents(InObject, OutEvents);
	}

	return OutEvents;
}

TArray<FActorIOFunction> UActorIOComponent::GetFunctionsForObject(AActor* InObject)
{
	TArray<FActorIOFunction> OutFunctions = UActorIOComponent::GetNativeFunctionsForObject(InObject);
	if (InObject && InObject->Implements<UActorIOInterface>())
	{
		IActorIOInterface::Execute_RegisterIOFunctions(InObject, OutFunctions);
	}

	return OutFunctions;
}

FActorIOEvent UActorIOComponent::MakeIOEvent(UObject* ContextObject, FName EventId, FName EventDispatcherName)
{
	FActorIOEvent OutEvent = FActorIOEvent();
	OutEvent.SetId(EventId);
	OutEvent.SetBlueprintDelegate(ContextObject, EventDispatcherName);
	return OutEvent;
}

TArray<FActorIOEvent> UActorIOComponent::GetNativeEventsForObject(AActor* InObject)
{
	TArray<FActorIOEvent> OutEvents = TArray<FActorIOEvent>();
	if (!IsValid(InObject))
	{
		return OutEvents;
	}

	OutEvents.Add(FActorIOEvent()
		.SetId(TEXT("OnActorBeginOverlap"))
		.SetDisplayName(FText::FromString(TEXT("OnActorBeginOverlap")))
		.SetTooltipText(FText::FromString(TEXT("Event when something overlaps with the actor.")))
		.SetSparseDelegate(InObject, TEXT("OnActorBeginOverlap")));

	OutEvents.Add(FActorIOEvent()
		.SetId(TEXT("OnActorEndOverlap"))
		.SetDisplayName(FText::FromString(TEXT("OnActorEndOverlap")))
		.SetTooltipText(FText::FromString(TEXT("Event when something no longer overlaps with the actor.")))
		.SetSparseDelegate(InObject, TEXT("OnActorEndOverlap")));

	return OutEvents;
}

TArray<FActorIOFunction> UActorIOComponent::GetNativeFunctionsForObject(AActor* InObject)
{
	TArray<FActorIOFunction> OutFunctions = TArray<FActorIOFunction>();
	if (!IsValid(InObject))
	{
		return OutFunctions;
	}

	if (InObject->IsA<AEmitter>())
	{
		OutFunctions.Add(FActorIOFunction()
			.SetId(TEXT("AEmitter::Activate"))
			.SetDisplayName(FText::FromString(TEXT("Activate")))
			.SetTooltipText(FText::FromString(TEXT("Activate the particle system.")))
			.SetFunction(TEXT("Activate")));

		OutFunctions.Add(FActorIOFunction()
			.SetId(TEXT("AEmitter::Deactivate"))
			.SetDisplayName(FText::FromString(TEXT("Deactivate")))
			.SetTooltipText(FText::FromString(TEXT("Deactivate the particle system.")))
			.SetFunction(TEXT("Deactivate")));
	}

	OutFunctions.Add(FActorIOFunction()
		.SetId(TEXT("AActor::SetActorHiddenInGame"))
		.SetDisplayName(FText::FromString(TEXT("SetActorHiddenInGame")))
		.SetTooltipText(FText::FromString(TEXT("Changes actor hidden state.")))
		.SetFunction(TEXT("SetActorHiddenInGame")));

	OutFunctions.Add(FActorIOFunction()
		.SetId(TEXT("AActor::DestroyActor"))
		.SetDisplayName(FText::FromString(TEXT("DestroyActor")))
		.SetTooltipText(FText::FromString(TEXT("Destroy the actor.")))
		.SetFunction(TEXT("K2_DestroyActor")));

	return OutFunctions;
}

void UActorIOComponent::OnUnregister()
{
	UnbindActions();

	Super::OnUnregister();
}

// Copyright 2024 Horizon Games. All Rights Reserved.

#include "ActorIOLink.h"
#include "ActorIOComponent.h"

UActorIOLink::UActorIOLink()
{
	LinkedAction = FActorIOAction();
	bWasExecuted = false;
	ActionDelegate = FScriptDelegate();
}

void UActorIOLink::BindAction(const FActorIOAction& Action)
{
	LinkedAction = Action;

	UActorIOComponent* OwnerIOComponent = GetOwnerIOComponent();
	if (!OwnerIOComponent)
	{
		return;
	}

	const TArray<FActorIOEvent> ValidEvents = OwnerIOComponent->GetEvents();
	const FActorIOEvent* TargetEvent = ValidEvents.FindByKey(LinkedAction.SourceEvent);
	if (!TargetEvent)
	{
		return;
	}

	FMulticastScriptDelegate* TargetDelegate = TargetEvent->EventDelegate;
	if (TargetDelegate)
	{
		ActionDelegate = FScriptDelegate();
		ActionDelegate.BindUFunction(this, TEXT("ExecuteAction"));

		TargetDelegate->Add(ActionDelegate);
	}
}

void UActorIOLink::ClearAction()
{
	UActorIOComponent* OwnerIOComponent = GetOwnerIOComponent();
	if (!OwnerIOComponent)
	{
		return;
	}

	const TArray<FActorIOEvent> ValidEvents = OwnerIOComponent->GetEvents();
	const FActorIOEvent* TargetEvent = ValidEvents.FindByKey(LinkedAction.SourceEvent);
	if (!TargetEvent)
	{
		return;
	}

	FMulticastScriptDelegate* TargetDelegate = TargetEvent->EventDelegate;
	if (TargetDelegate)
	{
		if (TargetDelegate->Contains(ActionDelegate))
		{
			TargetDelegate->Remove(ActionDelegate);
			ActionDelegate.Unbind();
		}
	}
}

void UActorIOLink::ExecuteAction()
{
	AActor* TargetActor = LinkedAction.ResolveTargetActorReference(GetWorld());
	if (!IsValid(TargetActor))
	{
		// The target actor was invalid.
		// Actor was most likely destroyed.
		return;
	}

	UFunction* TargetFunction = TargetActor->FindFunction(LinkedAction.TargetFunction);
	if (!TargetFunction)
	{
		return;
	}

	// #TODO: Handle function params.
	TargetActor->ProcessEvent(TargetFunction, nullptr);

	bWasExecuted = true;
}

UActorIOComponent* UActorIOLink::GetOwnerIOComponent() const
{
	// ActorIO links are owned by ActorIO components.
	return Cast<UActorIOComponent>(GetOuter());
}

void UActorIOLink::BeginDestroy()
{
	ClearAction();

	Super::BeginDestroy();
}

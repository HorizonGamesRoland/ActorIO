// Copyright 2024 Horizon Games. All Rights Reserved.

#include "ActorIOComponent.h"
#include "ActorIOInterface.h"

void UActorIOComponent::OnRegister()
{
	Super::OnRegister();

	InitializeMappings();

	UWorld* MyWorld = GetWorld();
	if (!MyWorld || !MyWorld->IsGameWorld())
	{
		// Do nothing in the editor (e.g. blueprint editor).
		return;
	}

	// #TEMP: Test action
	FActorIOAction NewAction = FActorIOAction();
	NewAction.SourceEvent = TEXT("TestEvent");
	NewAction.TargetActor = GetOwner();
	NewAction.TargetFunction = TEXT("Something");
	AddAction(NewAction);

	for (const FActorIOAction& Action : ActionBindings)
	{
		BindAction(Action);
	}

	// #TEMP: Raise the test event
	TestEvent.Broadcast(0);
}

void UActorIOComponent::InitializeMappings()
{
	MappedEvents.Reset();
	MappedFunctions.Reset();

	// #TEMP: Register self test event
	MappedEvents.Emplace(TEXT("TestEvent"), &TestEvent);

	IActorIOInterface* OwnerIO = Cast<IActorIOInterface>(GetOwner());
	if (OwnerIO)
	{
		OwnerIO->GetActorIOEvents(MappedEvents);
		OwnerIO->GetActorIOFunctions(MappedFunctions);
	}
}

void UActorIOComponent::AddAction(const FActorIOAction& NewAction)
{
	if (!NewAction.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ActorIOComponent: AddAction failed - The action was invalid!"));
		return;
	}

	FActorIOAction* ExistingAction = FindAction(NewAction.ActionId);
	if (ExistingAction)
	{
		UE_LOG(LogTemp, Error, TEXT("ActorIOComponent: AddAction failed - The action already exists!"));
		return;
	}

	ActionBindings.Add(NewAction);
}

void UActorIOComponent::RemoveAction(const FGuid& ActionId)
{
	// ...
}

FActorIOAction* UActorIOComponent::FindAction(const FGuid& ActionId)
{
	return ActionBindings.FindByKey(ActionId);
}

void UActorIOComponent::BindAction(const FActorIOAction& Action)
{
	FActorIOEvent* TargetEvent = MappedEvents.FindByKey(Action.SourceEvent);
	if (!TargetEvent)
	{
		return;
	}

	FMulticastScriptDelegate* TargetDelegate = TargetEvent->EventDelegate;
	if (TargetDelegate)
	{
		FScriptDelegate NewDelegate;
		NewDelegate.BindUFunction(this, TEXT("SomethingOnTestEvent"));
	
		TargetDelegate->Add(NewDelegate);
	}
}

void UActorIOComponent::RemoveActionBindings()
{
	// ...
}

void UActorIOComponent::SomethingOnTestEvent()
{
	UE_LOG(LogTemp, Warning, TEXT("SomethingOnTestEvent called!"));
}

void UActorIOComponent::OnUnregister()
{
	Super::OnUnregister();

	RemoveActionBindings();
}

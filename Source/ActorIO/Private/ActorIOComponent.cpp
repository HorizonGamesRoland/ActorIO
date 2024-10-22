// Copyright 2024 Horizon Games. All Rights Reserved.

#include "ActorIOComponent.h"
#include "ActorIOLink.h"
#include "ActorIOInterface.h"

UActorIOComponent::UActorIOComponent()
{
	Actions = TArray<FActorIOAction>();
	ActionBindings = TArray<TObjectPtr<UActorIOLink>>();
}

void UActorIOComponent::OnRegister()
{
	Super::OnRegister();

	UWorld* MyWorld = GetWorld();
	if (!MyWorld || !MyWorld->IsGameWorld())
	{
		// Do nothing in the editor (e.g level editor, blueprint).
		return;
	}

	// #TEMP: Test action
	FActorIOAction NewAction = FActorIOAction();
	NewAction.SourceEvent = TEXT("TestEvent");
	NewAction.TargetActor = GetOwner();
	NewAction.TargetFunction = TEXT("TestHelloWorld");
	Actions.Add(NewAction);

	CreateActionBindings();

	// #TEMP: Raise the test event
	//TestEvent.Broadcast(0);
}

const TArray<FActorIOEvent> UActorIOComponent::GetEvents() const
{
	TArray<FActorIOEvent> OutEvents = TArray<FActorIOEvent>();
	IActorIOInterface* OwnerIO = Cast<IActorIOInterface>(GetOwner());
	if (OwnerIO)
	{
		OwnerIO->GetActorIOEvents(OutEvents);
	}

	return OutEvents;
}

const TArray<FActorIOFunction> UActorIOComponent::GetFunctions() const
{
	TArray<FActorIOFunction> OutFunctions = TArray<FActorIOFunction>();
	IActorIOInterface* OwnerIO = Cast<IActorIOInterface>(GetOwner());
	if (OwnerIO)
	{
		OwnerIO->GetActorIOFunctions(OutFunctions);
	}



	// #TEMP: Inject test function.
	OutFunctions.Emplace(TEXT("TestHelloWorld"), TEXT("TestHelloWorld"));


	return OutFunctions;
}

void UActorIOComponent::CreateActionBindings()
{
	const int32 NumActions = Actions.Num();
	if (NumActions == 0)
	{
		return;
	}

	TArray<FActorIOEvent> ValidEvents = GetEvents();
	TArray<FActorIOFunction> ValidFunctions = GetFunctions();

	ActionBindings.Init(nullptr, NumActions);
	for (int32 ActionIdx = 0; ActionIdx != NumActions; ++ActionIdx)
	{
		ActionBindings[ActionIdx] = NewObject<UActorIOLink>(this);
		ActionBindings[ActionIdx]->BindAction(Actions[ActionIdx]);
	}
}

void UActorIOComponent::RemoveActionBindings()
{
	for (UActorIOLink* Link : ActionBindings)
	{
		if (IsValid(Link))
		{
			Link->ClearAction();
			Link->MarkAsGarbage();
		}
	}
}

void UActorIOComponent::TestHelloWorld()
{
	UE_LOG(LogTemp, Warning, TEXT("Hello World!"));
}

void UActorIOComponent::OnUnregister()
{
	RemoveActionBindings();

	Super::OnUnregister();
}

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

	CreateActionBindings();
}

const TArray<FActorIOEvent> UActorIOComponent::GetEvents() const
{
	TArray<FActorIOEvent> OutEvents = TArray<FActorIOEvent>();
	OutEvents.Append(GetNativeEventsForObject(GetOwner()));

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
	OutFunctions.Append(GetNativeFunctionsForObject(GetOwner()));

	IActorIOInterface* OwnerIO = Cast<IActorIOInterface>(GetOwner());
	if (OwnerIO)
	{
		OwnerIO->GetActorIOFunctions(OutFunctions);
	}

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

TArray<FActorIOEvent> UActorIOComponent::GetNativeEventsForObject(UObject* InObject)
{
	check(InObject);
	TArray<FActorIOEvent> OutEvents = TArray<FActorIOEvent>();

	if (InObject->IsA<AActor>())
	{
		OutEvents.Emplace(ToName(EActorIONativeEvents::ActorBeginOverlap), nullptr);
		OutEvents.Emplace(ToName(EActorIONativeEvents::ActorEndOverlap), nullptr);
	}

	return OutEvents;
}

TArray<FActorIOFunction> UActorIOComponent::GetNativeFunctionsForObject(UObject* InObject)
{
	check(InObject);
	TArray<FActorIOFunction> OutFunctions = TArray<FActorIOFunction>();

	if (InObject->IsA<AActor>())
	{
		OutFunctions.Emplace(ToName(EActorIONativeFunctions::SetActorHiddenInGame), TEXT("SetActorHiddenInGame"));
	}

	return OutFunctions;
}

void UActorIOComponent::OnUnregister()
{
	RemoveActionBindings();

	Super::OnUnregister();
}

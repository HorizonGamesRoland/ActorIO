// Copyright 2024 Horizon Games. All Rights Reserved.

#include "ActorIOComponent.h"
#include "ActorIOAction.h"
#include "ActorIOInterface.h"
#include "GameFramework/Actor.h"

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

void UActorIOComponent::CreateActionBindings()
{
	const int32 NumActions = Actions.Num();
	if (NumActions == 0)
	{
		return;
	}

	TArray<FActorIOEvent> ValidEvents = UActorIOComponent::GetEventsForObject(GetOwner());
	TArray<FActorIOFunction> ValidFunctions = UActorIOComponent::GetFunctionsForObject(GetOwner());

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

TArray<FActorIOEvent> UActorIOComponent::GetEventsForObject(AActor* InObject)
{
	TArray<FActorIOEvent> OutEvents = UActorIOComponent::GetNativeEventsForObject(InObject);
	IActorIOInterface* TargetIO = Cast<IActorIOInterface>(InObject);
	if (TargetIO)
	{
		TargetIO->GetActorIOEvents(OutEvents);
	}

	return OutEvents;
}

TArray<FActorIOFunction> UActorIOComponent::GetFunctionsForObject(AActor* InObject)
{
	TArray<FActorIOFunction> OutFunctions = UActorIOComponent::GetNativeFunctionsForObject(InObject);
	IActorIOInterface* TargetIO = Cast<IActorIOInterface>(InObject);
	if (TargetIO)
	{
		TargetIO->GetActorIOFunctions(OutFunctions);
	}

	return OutFunctions;
}

TArray<FActorIOEvent> UActorIOComponent::GetNativeEventsForObject(AActor* InObject)
{
	TArray<FActorIOEvent> OutEvents = TArray<FActorIOEvent>();
	if (!IsValid(InObject))
	{
		return OutEvents;
	}

	if (InObject->IsA<AActor>())
	{
		OutEvents.Add(FActorIOEvent()
			.SetId(TEXT("OnActorBeginOverlap"))
			.SetDisplayName(FText::FromString(TEXT("OnActorBeginOverlap")))
			.SetTooltipText(FText::FromString(TEXT("Event when something overalps with the actor.")))
			.SetSparseDelegate(InObject, TEXT("OnActorBeginOverlap")));

		OutEvents.Add(FActorIOEvent()
			.SetId(TEXT("OnActorEndOverlap"))
			.SetDisplayName(FText::FromString(TEXT("OnActorEndOverlap")))
			.SetTooltipText(FText::FromString(TEXT("Event when something no longer overalps with the actor.")))
			.SetSparseDelegate(InObject, TEXT("OnActorEndOverlap")));
	}

	return OutEvents;
}

TArray<FActorIOFunction> UActorIOComponent::GetNativeFunctionsForObject(AActor* InObject)
{
	TArray<FActorIOFunction> OutFunctions = TArray<FActorIOFunction>();
	if (!IsValid(InObject))
	{
		return OutFunctions;
	}

	if (InObject->IsA<AActor>())
	{
		OutFunctions.Add(FActorIOFunction()
			.SetId(TEXT("SetActorHiddenInGame"))
			.SetDisplayName(FText::FromString(TEXT("SetActorHiddenInGame")))
			.SetTooltipText(FText::FromString(TEXT("Changes actor hidden state.")))
			.SetFunction(TEXT("SetActorHiddenInGame")));
	}

	return OutFunctions;
}

void UActorIOComponent::OnUnregister()
{
	RemoveActionBindings();

	Super::OnUnregister();
}

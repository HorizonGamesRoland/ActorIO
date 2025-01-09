// Copyright 2024 Horizon Games. All Rights Reserved.

#include "ActorIOComponent.h"
#include "ActorIOAction.h"

UActorIOComponent::UActorIOComponent()
{
	Actions = TArray<TObjectPtr<UActorIOAction>>();
}

void UActorIOComponent::OnRegister()
{
	Super::OnRegister();

	RemoveInvalidActions();

	UWorld* MyWorld = GetWorld();
	if (MyWorld && MyWorld->IsGameWorld())
	{
		AActor* MyOwner = GetOwner();
		if (!MyOwner->IsActorTickEnabled())
		{
			MyOwner->SetActorTickEnabled(true);
		}

		BindActions();
	}
}

UActorIOAction* UActorIOComponent::CreateNewAction()
{
	const FName ObjectName = MakeUniqueObjectName(this, UActorIOAction::StaticClass());
	UActorIOAction* NewAction = NewObject<UActorIOAction>(this, ObjectName, RF_Transactional);
	Actions.Add(NewAction);
	return NewAction;
}

void UActorIOComponent::RemoveAction(UActorIOAction* InAction)
{
	check(InAction);
	InAction->MarkAsGarbage();

	const int32 ActionIdx = Actions.IndexOfByKey(InAction);
	Actions.RemoveAt(ActionIdx);
}

void UActorIOComponent::RemoveInvalidActions()
{
	for (int32 ActionIdx = Actions.Num() - 1; ActionIdx >= 0; --ActionIdx)
	{
		if (!Actions[ActionIdx].Get())
		{
			Actions.RemoveAt(ActionIdx);
		}
	}
}

TArray<TWeakObjectPtr<UActorIOAction>> UActorIOComponent::GetActions() const
{
	TArray<TWeakObjectPtr<UActorIOAction>> OutActions = TArray<TWeakObjectPtr<UActorIOAction>>();
	for (UActorIOAction* Action : Actions)
	{
		if (IsValid(Action))
		{
			OutActions.Add(Action);
		}
	}

	return OutActions;
}

float UActorIOComponent::GetDurationOfLongestDelay() const
{
	float OutLongestDelay = 0.0f;
	for (int32 ActionIdx = 0; ActionIdx != Actions.Num(); ++ActionIdx)
	{
		if (IsValid(Actions[ActionIdx]) && Actions[ActionIdx]->Delay > OutLongestDelay)
		{
			OutLongestDelay = Actions[ActionIdx]->Delay;
		}
	}

	return OutLongestDelay;
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
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	for (int32 ActionIdx = 0; ActionIdx != Actions.Num(); ++ActionIdx)
	{
		UActorIOAction* Action = Actions[ActionIdx].Get();
		if (Action)
		{
			Action->UnbindAction();
			TimerManager.ClearAllTimersForObject(Action);
		}
	}
}

void UActorIOComponent::OnUnregister()
{
	UWorld* MyWorld = GetWorld();
	if (MyWorld && MyWorld->IsGameWorld())
	{
		UnbindActions();
	}

	Super::OnUnregister();
}

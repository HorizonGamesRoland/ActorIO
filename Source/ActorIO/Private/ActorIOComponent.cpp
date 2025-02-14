// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOComponent.h"
#include "ActorIOAction.h"
#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"

#define LOCTEXT_NAMESPACE "ActorIO"

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
	UActorIOAction* NewAction = NewObject<UActorIOAction>(this, NAME_None, RF_Transactional);
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

float UActorIOComponent::GetDurationOfLongestDelay() const
{
	float OutLongestDelay = 0.0f;
	for (int32 ActionIdx = 0; ActionIdx != Actions.Num(); ++ActionIdx)
	{
		UActorIOAction* Action = Actions[ActionIdx].Get();
		if (IsValid(Action) && Action->Delay > OutLongestDelay)
		{
			OutLongestDelay = Action->Delay;
		}
	}

	return OutLongestDelay;
}

void UActorIOComponent::BindActions()
{
	for (int32 ActionIdx = 0; ActionIdx != Actions.Num(); ++ActionIdx)
	{
		UActorIOAction* Action = Actions[ActionIdx].Get();
		if (IsValid(Action))
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
		if (IsValid(Action))
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

#if WITH_EDITOR
void UActorIOComponent::CheckForErrors()
{
	Super::CheckForErrors();

	AActor* Owner = GetOwner();

	const int32 NumActions = Actions.Num();
	if (NumActions > 0)
	{
		const FActorIOEventList ValidEvents = IActorIO::GetEventsForObject(Owner);
		const FActorIOFunctionList ValidFunctions = IActorIO::GetFunctionsForObject(Owner);

		for (int32 ActionIdx = 0; ActionIdx != NumActions; ++ActionIdx)
		{
			const UActorIOAction* Action = Actions[ActionIdx].Get();
			if (!IsValid(Action))
			{
				FMessageLog("MapCheck").Error()
					->AddToken(FTextToken::Create(LOCTEXT("MapCheck_Message_IOPrefix", "[I/O]")))
					->AddToken(FUObjectToken::Create(Owner))
					->AddToken(FTextToken::Create(LOCTEXT("MapCheck_Message_IOComponentInvalidAction", "contains a null action. Re-open the level to fix.")));
			}
			else
			{
				const FActorIOEvent* TargetEvent = ValidEvents.GetEvent(Action->EventId);
				if (!TargetEvent)
				{
					FMessageLog("MapCheck").Warning()
						->AddToken(FTextToken::Create(LOCTEXT("MapCheck_Message_IOPrefix", "[I/O]")))
						->AddToken(FUObjectToken::Create(Owner))
						->AddToken(FTextToken::Create(LOCTEXT("MapCheck_Message_IOActionInvalidEvent", "contains an action with invalid event selected.")));
				}

				const FActorIOFunction* TargetFunction = ValidFunctions.GetFunction(Action->FunctionId);
				if (!TargetFunction)
				{
					FMessageLog("MapCheck").Warning()
						->AddToken(FTextToken::Create(LOCTEXT("MapCheck_Message_IOPrefix", "[I/O]")))
						->AddToken(FUObjectToken::Create(Owner))
						->AddToken(FTextToken::Create(LOCTEXT("MapCheck_Message_IOActionInvalidFunction", "contains an action with invalid target function selected.")));
				}

				if (!IsValid(Action->TargetActor))
				{
					FMessageLog("MapCheck").Warning()
						->AddToken(FTextToken::Create(LOCTEXT("MapCheck_Message_IOPrefix", "[I/O]")))
						->AddToken(FUObjectToken::Create(Owner))
						->AddToken(FTextToken::Create(LOCTEXT("MapCheck_Message_IOActionInvalidTarget", "contains an action with invalid target actor selected.")));
				}
			}
		}
	}
}
#endif

#undef LOCTEXT_NAMESPACE

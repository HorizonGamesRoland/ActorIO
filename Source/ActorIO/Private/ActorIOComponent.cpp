// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOComponent.h"
#include "ActorIOAction.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"

#define LOCTEXT_NAMESPACE "ActorIO"

UActorIOComponent::UActorIOComponent()
{
	// Required for component initialize/deinitialize callbacks.
	bWantsInitializeComponent = true;

	Actions = TArray<TObjectPtr<UActorIOAction>>();
}

void UActorIOComponent::OnRegister()
{
	Super::OnRegister();

	// Clean up the action list whenever the component is (re)registered.
	CompactActions();
}

void UActorIOComponent::InitializeComponent()
{
	Super::InitializeComponent();

	BindActions();
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

void UActorIOComponent::MoveAction(int32 OriginalIndex, int32 NewIndex)
{
	// Uses same move implementation as properties in the editor.
	// @see FPropertyValueImpl::MoveElementTo

	if (NewIndex > OriginalIndex)
	{
		Actions.InsertDefaulted(NewIndex + 1);
		Actions.Swap(OriginalIndex, NewIndex + 1);
		Actions.RemoveAt(OriginalIndex);
	}
	else if (NewIndex < OriginalIndex)
	{
		Actions.InsertDefaulted(NewIndex);
		Actions.Swap(OriginalIndex + 1, NewIndex);
		Actions.RemoveAt(OriginalIndex + 1);
	}
}

void UActorIOComponent::CompactActions()
{
	for (int32 ActionIdx = Actions.Num() - 1; ActionIdx >= 0; --ActionIdx)
	{
		if (!Actions[ActionIdx].Get())
		{
			Actions.RemoveAt(ActionIdx);
		}
	}
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
	for (int32 ActionIdx = 0; ActionIdx != Actions.Num(); ++ActionIdx)
	{
		UActorIOAction* Action = Actions[ActionIdx].Get();
		if (IsValid(Action))
		{
			Action->UnbindAction();
		}
	}
}

void UActorIOComponent::UninitializeComponent()
{
	UnbindActions();

	Super::UninitializeComponent();
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

		for (int32 ActionIdx = 0; ActionIdx != NumActions; ++ActionIdx)
		{
			const UActorIOAction* ActionPtr = Actions[ActionIdx].Get();
			if (!IsValid(ActionPtr))
			{
				FMessageLog("MapCheck").Error()
					->AddToken(FTextToken::Create(LOCTEXT("MapCheck_Message_IOPrefix", "[I/O]")))
					->AddToken(FUObjectToken::Create(Owner))
					->AddToken(FTextToken::Create(LOCTEXT("MapCheck_Message_IOComponentInvalidAction", "contains a null action. Re-open the level to fix.")));
			}
			else
			{
				const FActorIOEvent* TargetEvent = ValidEvents.GetEvent(ActionPtr->EventId);
				if (!TargetEvent)
				{
					FMessageLog("MapCheck").Warning()
						->AddToken(FTextToken::Create(LOCTEXT("MapCheck_Message_IOPrefix", "[I/O]")))
						->AddToken(FUObjectToken::Create(Owner))
						->AddToken(FTextToken::Create(LOCTEXT("MapCheck_Message_IOActionInvalidEvent", "contains an action with invalid event selected.")));
				}

				if (ActionPtr->TargetActor.IsNull())
				{
					FMessageLog("MapCheck").Warning()
						->AddToken(FTextToken::Create(LOCTEXT("MapCheck_Message_IOPrefix", "[I/O]")))
						->AddToken(FUObjectToken::Create(Owner))
						->AddToken(FTextToken::Create(LOCTEXT("MapCheck_Message_IOActionMissingTarget", "contains an action with no target actor selected.")));
				}
				else if (ActionPtr->TargetActor.IsValid())
				{
					const FActorIOFunctionList ValidFunctions = IActorIO::GetFunctionsForObject(ActionPtr->TargetActor.Get());
					const FActorIOFunction* TargetFunction = ValidFunctions.GetFunction(ActionPtr->FunctionId);
					if (!TargetFunction)
					{
						FMessageLog("MapCheck").Warning()
							->AddToken(FTextToken::Create(LOCTEXT("MapCheck_Message_IOPrefix", "[I/O]")))
							->AddToken(FUObjectToken::Create(Owner))
							->AddToken(FTextToken::Create(LOCTEXT("MapCheck_Message_IOActionInvalidFunction", "contains an action with invalid target function selected.")));
					}

					UFunction* FunctionPtr = ActionPtr->ResolveUFunction();
					if (FunctionPtr)
					{
						FText ErrorText = FText::GetEmpty();
						if (!IActorIO::ValidateFunctionArguments(FunctionPtr, ActionPtr->FunctionArguments, ErrorText))
						{
							FMessageLog("MapCheck").Warning()
								->AddToken(FTextToken::Create(LOCTEXT("MapCheck_Message_IOPrefix", "[I/O]")))
								->AddToken(FUObjectToken::Create(Owner))
								->AddToken(FTextToken::Create(LOCTEXT("MapCheck_Message_IOActionInvalidArguments", "contains an action with invalid function parameters.")));
						}
					}
				}
			}
		}
	}
}
#endif

#undef LOCTEXT_NAMESPACE

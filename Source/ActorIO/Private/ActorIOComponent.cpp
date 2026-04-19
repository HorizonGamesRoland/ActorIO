// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOComponent.h"
#include "ActorIOAction.h"
#include "ActorIOVersions.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Serialization/Formatters/BinaryArchiveFormatter.h"
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

void UActorIOComponent::SerializeToRawData(TArray<uint8>& RawData)
{
	FMemoryWriter Archive = FMemoryWriter(RawData);
	FObjectAndNameAsStringProxyArchive ProxyArchive = FObjectAndNameAsStringProxyArchive(Archive, false);
	ProxyArchive.ArIsSaveGame = true;

	FBinaryArchiveFormatter Formatter = FBinaryArchiveFormatter(ProxyArchive);
	FStructuredArchive StructuredArchive = FStructuredArchive(Formatter);

	FStructuredArchive::FSlot RootSlot = StructuredArchive.Open();
	Serialize(RootSlot.EnterRecord());
}

void UActorIOComponent::RestoreFromRawData(TArray<uint8>& RawData)
{
	FMemoryReader Archive = FMemoryReader(RawData);
	FObjectAndNameAsStringProxyArchive ProxyArchive = FObjectAndNameAsStringProxyArchive(Archive, false);
	ProxyArchive.ArIsSaveGame = true;

	FBinaryArchiveFormatter Formatter = FBinaryArchiveFormatter(ProxyArchive);
	FStructuredArchive StructuredArchive = FStructuredArchive(Formatter);

	FStructuredArchive::FSlot RootSlot = StructuredArchive.Open();
	Serialize(RootSlot.EnterRecord());
}

void UActorIOComponent::UninitializeComponent()
{
	UnbindActions();

	Super::UninitializeComponent();
}

void UActorIOComponent::Serialize(FStructuredArchive::FRecord Record)
{
	FArchive& UnderlyingArchive = Record.GetUnderlyingArchive();
	if (UnderlyingArchive.IsSaveGame())
	{
		UnderlyingArchive.UsingCustomVersion(FActorIOActionVersion::GUID);

		int32 Version = UnderlyingArchive.CustomVer(FActorIOActionVersion::GUID);
		Record << SA_VALUE(TEXT("Version"), Version);

		if (UnderlyingArchive.IsLoading())
		{
			UnderlyingArchive.SetCustomVersion(FActorIOActionVersion::GUID, Version, TEXT("ActorIOActionVer"));
		}

		TArray<UActorIOAction*> SerializeActions;
		if (UnderlyingArchive.IsSaving())
		{
			for (const TObjectPtr<UActorIOAction>& ActionPtr : Actions)
			{
				UActorIOAction* Action = ActionPtr.Get();
				if (Action && Action->ShouldSerializeToArchive(UnderlyingArchive))
				{
					SerializeActions.Add(Action);
				}
			}
		}

		int32 NumActions = SerializeActions.Num();
		FStructuredArchive::FMap ActionsMap = Record.EnterMap(TEXT("Actions"), NumActions);

		for (int32 ActionIdx = 0; ActionIdx != NumActions; ++ActionIdx)
		{
			UActorIOAction* ActionPtr = nullptr;
			FString ActionName;

			if (UnderlyingArchive.IsSaving())
			{
				ActionPtr = SerializeActions[ActionIdx];
				ActionName = ActionPtr->GetName();
			}

			FStructuredArchive::FSlot ActionSlot = ActionsMap.EnterElement(ActionName);
			FStructuredArchive::FRecord ActionRecord = ActionSlot.EnterRecord();

			const int64 DataSizePosition = UnderlyingArchive.Tell();
			int64 DataSize = 0;

			// Pre-serialize the data size. We'll rewrite this after serializing the action.
			if (!UnderlyingArchive.IsTextFormat())
			{
				ActionRecord << SA_VALUE(TEXT("DataSize"), DataSize);
			}

			const int64 BeginDataPosition = UnderlyingArchive.Tell();

			if (UnderlyingArchive.IsLoading())
			{
				ActionPtr = FindObjectFast<UActorIOAction>(this, *ActionName);
				UE_CLOG(!ActionPtr, LogActorIO, Warning, TEXT("%s - No action found with name '%s'."), *GetPathName(), *ActionName);

				if (ActionPtr)
				{
					ActionPtr->Serialize(ActionRecord);
				}
				
				// Seek to the end of the data block in case we serialized less.
				if (!UnderlyingArchive.IsTextFormat())
				{
					if (!ensureMsgf(UnderlyingArchive.Tell() <= BeginDataPosition + DataSize, TEXT("%s - Serialized more data then expected when loading %s!"), *GetPathName(), *ActionName))
					{
						UnderlyingArchive.SetError();
						return;
					}

					UnderlyingArchive.Seek(BeginDataPosition + DataSize);
				}
			}
			else
			{
				check(ActionPtr);
				ActionPtr->Serialize(ActionRecord);

				// Seek back and re-write the data size with the actual size.
				if (!UnderlyingArchive.IsTextFormat())
				{
					const int64 EndDataPosition = UnderlyingArchive.Tell();
					DataSize = EndDataPosition - BeginDataPosition;

					UnderlyingArchive.Seek(DataSizePosition);
					UnderlyingArchive << DataSize;
					UnderlyingArchive.Seek(EndDataPosition);
				}
			}
		}
	}
	else
	{
		// Use the default property serialization if not saving game data.
		Super::Serialize(Record);
	}
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

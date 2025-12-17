// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOAction.h"
#include "ActorIOComponent.h"
#include "ActorIOInterface.h"
#include "ActorIOSubsystemBase.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

FName UActorIOAction::NAME_ExecuteAction(TEXT("ExecuteAction"));

UActorIOAction::UActorIOAction()
{
	EventId = NAME_None;
	TargetActor = nullptr;
	FunctionId = NAME_None;
	FunctionArguments = FString();
	Delay = 0.0f;
	bExecuteOnlyOnce = false;

	bWasExecuted = false;
	bIsBound = false;
	ActionDelegate = FScriptDelegate();
}

void UActorIOAction::BindAction()
{
	AActor* ActionOwner = GetOwnerActor();
	check(ActionOwner);

	if (bIsBound)
	{
		UE_CLOG(DebugIOActions, LogActorIO, Error, TEXT("Actor '%s' could not bind action to '%s' - Action is already bound!"), *ActionOwner->GetActorNameOrLabel(), *EventId.ToString());
		return;
	}

	const FActorIOEventList ValidEvents = IActorIO::GetEventsForObject(ActionOwner);
	const FActorIOEvent* TargetEvent = ValidEvents.GetEvent(EventId);
	if (!TargetEvent)
	{
		UE_CLOG(DebugIOActions, LogActorIO, Error, TEXT("Actor '%s' could not bind action to '%s' - Event was not found."), *ActionOwner->GetActorNameOrLabel(), *EventId.ToString());
		return;
	}

	UObject* DelegateOwner = TargetEvent->DelegateOwner.Get();
	if (!IsValid(DelegateOwner))
	{
		UE_CLOG(DebugIOActions, LogActorIO, Error, TEXT("Actor '%s' could not bind action to '%s' - Delegate owner was invalid."), *ActionOwner->GetActorNameOrLabel(), *EventId.ToString());
		return;
	}

	ActionDelegate = FScriptDelegate();
	ActionDelegate.BindUFunction(this, NAME_ExecuteAction);

	switch (TargetEvent->DelegateType)
	{
		// Binding to multicast delegate directly.
		// Since we have a direct reference to the delegate, we can simply add to it.
		case FActorIOEvent::Type::MulticastDelegate:
		{
			if (TargetEvent->MulticastDelegatePtr)
			{
				TargetEvent->MulticastDelegatePtr->Add(ActionDelegate);
				bIsBound = true;
			}

			UE_CLOG(DebugIOActions && !bIsBound, LogActorIO, Error, TEXT("Actor '%s' could not bind action to '%s' - Delegate reference was nullptr."), *ActionOwner->GetActorNameOrLabel(), *EventId.ToString());
			break;
		}
		
		// Binding to a sparse delegate.
		// These delegates are stored in a global storage so we need to resolve it first.
		// Then we have to use the interal add function because there's no other way to set the bIsBound param for it.
		// If the bIsBound param is not set, the delegate will not execute.
		case FActorIOEvent::Type::SparseDelegate:
		{
			FSparseDelegate* SparseDelegate = FSparseDelegateStorage::ResolveSparseDelegate(DelegateOwner, TargetEvent->SparseDelegateName);
			if (SparseDelegate)
			{
				SparseDelegate->__Internal_AddUnique(DelegateOwner, TargetEvent->SparseDelegateName, ActionDelegate);
				bIsBound = true;
			}

			UE_CLOG(DebugIOActions && !bIsBound, LogActorIO, Error, TEXT("Actor '%s' could not bind action to '%s' - Failed to resolve sparse delegate with name '%s'."), *ActionOwner->GetActorNameOrLabel(), *EventId.ToString(), *TargetEvent->SparseDelegateName.ToString());
			break;
		}
		
		// Binding to a blueprint exposed dynamic delegate.
		// These include event dispatchers created in blueprints.
		// Each event dispatcher is basically just an FMulticastDelegateProperty that we can add to.
		case FActorIOEvent::Type::BlueprintDelegate:
		{
			UClass* DelegateOwnerClass = DelegateOwner->GetClass();
			FMulticastDelegateProperty* DelegateProp = CastField<FMulticastDelegateProperty>(DelegateOwnerClass->FindPropertyByName(TargetEvent->BlueprintDelegateName));
			if (DelegateProp)
			{
				DelegateProp->AddDelegate(ActionDelegate, DelegateOwner);
				bIsBound = true;
			}

			UE_CLOG(DebugIOActions && !bIsBound, LogActorIO, Error, TEXT("Actor '%s' could not bind action to '%s' - No event dispatcher found with name '%s'."), *ActionOwner->GetActorNameOrLabel(), *EventId.ToString(), *TargetEvent->BlueprintDelegateName.ToString());
			break;
		}

		case FActorIOEvent::Type::Null:
		{
			UE_CLOG(DebugIOActions, LogActorIO, Error, TEXT("Actor '%s' could not bind action to '%s' - Delegate type was null! Forgot to set a delegate?"), *ActionOwner->GetActorNameOrLabel(), *EventId.ToString(), *TargetEvent->BlueprintDelegateName.ToString());
			break;
		}
	}
}

void UActorIOAction::UnbindAction()
{
	AActor* ActionOwner = GetOwnerActor();
	check(ActionOwner);

	if (!bIsBound)
	{
		return;
	}

	UActorIOComponent* OwnerIOComponent = GetOwnerIOComponent();
	if (!OwnerIOComponent)
	{
		return;
	}

	const FActorIOEventList ValidEvents = IActorIO::GetEventsForObject(OwnerIOComponent->GetOwner());
	const FActorIOEvent* TargetEvent = ValidEvents.GetEvent(EventId);
	if (!TargetEvent)
	{
		// This should be impossible to reach.
		// Basically the I/O event that the action is bound to was not found.
		// Only case when this can happen is if your register IO events function does not always return the same list of events.
		checkf(false, TEXT("Could not unbind action because the I/O event that we were bound to was not found?!"));
	}

	UObject* DelegateOwner = TargetEvent->DelegateOwner.Get(true);
	if (!DelegateOwner)
	{
		UE_CLOG(DebugIOActions, LogActorIO, Error, TEXT("Actor '%s' could not unbind action from '%s' - Delegate owner was nullptr."), *ActionOwner->GetActorNameOrLabel(), *EventId.ToString());
		return;
	}

	switch (TargetEvent->DelegateType)
	{
		case FActorIOEvent::Type::MulticastDelegate:
		{
			FMulticastScriptDelegate* TargetDelegate = TargetEvent->MulticastDelegatePtr;
			if (TargetDelegate)
			{
				TargetDelegate->Remove(ActionDelegate);
				bIsBound = false;
			}

			UE_CLOG(DebugIOActions && bIsBound, LogActorIO, Error, TEXT("Actor '%s' could not unbind action from '%s' - Delegate reference was nullptr."), *ActionOwner->GetActorNameOrLabel(), *EventId.ToString());
			break;
		}

		case FActorIOEvent::Type::SparseDelegate:
		{
			FSparseDelegate* SparseDelegate = FSparseDelegateStorage::ResolveSparseDelegate(DelegateOwner, TargetEvent->SparseDelegateName);
			if (SparseDelegate)
			{
				SparseDelegate->__Internal_Remove(DelegateOwner, TargetEvent->SparseDelegateName, ActionDelegate);
				bIsBound = false;
			}

			UE_CLOG(DebugIOActions && bIsBound, LogActorIO, Error, TEXT("Actor '%s' could not unbind action from '%s' - Failed to resolve sparse delegate with name '%s'."), *ActionOwner->GetActorNameOrLabel(), *EventId.ToString(), *TargetEvent->SparseDelegateName.ToString());
			break;
		}

		case FActorIOEvent::Type::BlueprintDelegate:
		{
			UClass* DelegateOwnerClass = DelegateOwner->GetClass();
			FMulticastDelegateProperty* DelegateProp = CastField<FMulticastDelegateProperty>(DelegateOwnerClass->FindPropertyByName(TargetEvent->BlueprintDelegateName));
			if (DelegateProp)
			{
				DelegateProp->RemoveDelegate(ActionDelegate, DelegateOwner);
				bIsBound = false;
			}

			UE_CLOG(DebugIOActions && bIsBound, LogActorIO, Error, TEXT("Actor '%s' could not unbind action from '%s' - No event dispatcher found with name '%s'."), *ActionOwner->GetActorNameOrLabel(), *EventId.ToString(), *TargetEvent->BlueprintDelegateName.ToString());
			break;
		}

		case FActorIOEvent::Type::Null:
		{
			// This should be impossible to reach.
			// Implies that the action was bound to an I/O event successfully before, but now the event reverted to null type.
			checkf(false, TEXT("Could not unbind action because the I/O event delegate type is null?!"));
		}
	}
}

void UActorIOAction::ProcessEvent(UFunction* Function, void* Parms)
{
	// This function is called whenever UnrealScript wants to execute a UFunction on this object.
	// We are going to use this to catch when 'execute action' is being called by the I/O event that we are bound to.
	// Since we have access to the original script VM memory here, we can process it ourselves.
	// Afterwards we let the script VM call our 'execute action' function, where we dispatch the I/O message.

	if (Function && Function->GetFName() == NAME_ExecuteAction)
	{
		FActionExecutionContext& ExecContext = FActionExecutionContext::Get(this);
		ExecContext.EnterContext(this, Parms);

		// Process the execute action call.
		ExecContext.bProcessResult = ProcessAction(ExecContext);
	}
	
	Super::ProcessEvent(Function, Parms);
}

bool UActorIOAction::ProcessAction(FActionExecutionContext& ExecutionContext)
{
	AActor* ActionOwner = GetOwnerActor();

	FString OwnerInvalidReason;
	if (!IActorIO::ConfirmObjectIsAlive(ActionOwner, OwnerInvalidReason))
	{
		// Do nothing if the owning actor is invalid.
		return false;
	}

	if (bExecuteOnlyOnce && bWasExecuted)
	{
		// Do nothing if execute only once is enabled and the action was executed already.
		return false;
	}

	UActorIOSubsystemBase* IOSubsystem = UActorIOSubsystemBase::Get(this);
	if (!IOSubsystem)
	{
		// Do nothing if the I/O subsystem is invalid.
		// This should be impossible to reach.
		return false;
	}

	UE_CLOG(DebugIOActions, LogActorIO, Log, TEXT("Executing action: %s -> %s (Caller: '%s')"), *EventId.ToString(), *FunctionId.ToString(), *ActionOwner->GetActorNameOrLabel());

	if (TargetActor.IsNull())
	{
		// Do nothing if no target actor is selected.
		IActorIO::ExecutionError(DebugIOActions && WarnIOInvalidTarget, ELogVerbosity::Warning, TEXT("No target actor selected."));
		return false;
	}

	IActorIOInterface* ActionOwnerIOInterface = nullptr;
	if (ActionOwner->Implements<UActorIOInterface>())
	{
		ActionOwnerIOInterface = Cast<IActorIOInterface>(ActionOwner);
	}

	const bool bProcessNamedArgs = FunctionArguments.Contains(NAMEDARGUMENT_PREFIX) || LogIONamedArgs;
	if (bProcessNamedArgs)
	{
		// Let the I/O subsystem add globally available named arguments to the current execution context.
		// Think stuff like reference to player character, or player controller.
		IOSubsystem->GetGlobalNamedArguments(ExecutionContext);

		// Let the owning actor add locally available named arguments to the current execution context.
		if (ActionOwnerIOInterface)
		{
			ActionOwnerIOInterface->GetLocalNamedArguments(ExecutionContext);
			IActorIOInterface::Execute_K2_GetLocalNamedArguments(ActionOwner);
		}
	}

	FActorIOEventList ValidEvents = IActorIO::GetEventsForObject(ActionOwner);
	FActorIOEvent* BoundEvent = ValidEvents.GetEvent(EventId);
	check(BoundEvent);

	// Run the event processor.
	// We are calling the event processor with the original params memory that we received from the delegate.
	// This way the event processor will receive the proper values for its params given that its signature matches the delegate.
	if (BoundEvent->EventProcessor.IsBound())
	{
		BoundEvent->EventProcessor.ProcessDelegate<UObject>(ExecutionContext.ScriptParams);
	}

	// Log named arguments to console for debugging if needed.
	if (LogIONamedArgs)
	{
		UE_LOG(LogActorIO, Log, TEXT("  Named Arguments: (%d)"), ExecutionContext.NamedArguments.Num());
		for (const TPair<FString, FString>& NamedArg : ExecutionContext.NamedArguments)
		{
			UE_LOG(LogActorIO, Log, TEXT("  - %s = %s"), *NamedArg.Key, *NamedArg.Value);
		}
	}

	// Give the owning actor a chance to abort action execution (if it wasn't aborted already by the event processor).
	// Deliberately doing this after collecting named arguments in case we want to access them.
	if (ActionOwnerIOInterface && !ExecutionContext.bAborted)
	{
		if (IActorIOInterface::Execute_ConditionalAbortIOAction(ActionOwner, this))
		{
			ExecutionContext.AbortAction();
		}
	}

	return !ExecutionContext.bAborted;
}

void UActorIOAction::ExecuteAction()
{
	FActionExecutionContext& ExecutionContext = FActionExecutionContext::Get(this);
	check(ExecutionContext.HasContext());

	// Early out if action was stopped or aborted.
	if (!ExecutionContext.bProcessResult)
	{
		UE_CLOG(DebugIOActions && ExecutionContext.bAborted, LogActorIO, Log, TEXT("Action was aborted."));
		ExecutionContext.ExitContext();
		return;
	}

	// Break up the user defined arguments string from a single line into multiple elements.
	// This is the text that was set by the user in the Actor I/O editor.
	TArray<FString> Arguments;
	if (FunctionArguments.ParseIntoArray(Arguments, ARGUMENT_SEPARATOR, true) > 0)
	{
		for (FString& Argument : Arguments)
		{
			// Remove all whitespaces unless they are between quotes.
			// This is because the final command uses whitespace as the separator.
			// Using reverse iteration to avoid issues with character deletion.
			bool bInQuote = false;
			for (int32 CharIndex = Argument.Len() - 1; CharIndex >= 0; --CharIndex)
			{
				if (Argument[CharIndex] == '"')
				{
					bInQuote = !bInQuote;
				}
				else if (FChar::IsWhitespace(Argument[CharIndex]) && !bInQuote)
				{
					Argument.RemoveAt(CharIndex);
				}
			}

			// Replace named arguments with their actual values.
			// Everything stays in string form until the very end when the final command is sent.
			// Argument values will be parsed by UnrealScript.
			if (Argument.StartsWith(NAMEDARGUMENT_PREFIX) && ExecutionContext.NamedArguments.Contains(Argument))
			{
				const FString NamedArgValue = ExecutionContext.NamedArguments[Argument];
				Argument = NamedArgValue;
			}
		}
	}

	// Merge the processed arguments back into a single string in UnrealScript command format.
	FString ProcessedArgs;
	for (const FString& Argument : Arguments)
	{
		ProcessedArgs.Append(TEXT(" "));
		ProcessedArgs.Append(Argument);
	}

	FActorIOMessage NewMessage;
	NewMessage.SenderPtr = this;
	NewMessage.TargetPtr = TargetActor;
	NewMessage.FunctionId = FunctionId;
	NewMessage.Arguments = ProcessedArgs;
	NewMessage.TimeRemaining = Delay;

	// Leave the context now because QueueMessage can immediately lead into another I/O execution.
	ExecutionContext.ExitContext();
	bWasExecuted = true;

	UActorIOSubsystemBase* IOSubsystem = UActorIOSubsystemBase::Get(this);
	IOSubsystem->QueueMessage(NewMessage);
}

UActorIOComponent* UActorIOAction::GetOwnerIOComponent() const
{
	// Actions are owned by the actor's I/O component.
	return Cast<UActorIOComponent>(GetOuter());
}

AActor* UActorIOAction::GetOwnerActor() const
{
	UActorIOComponent* OwnerComponent = GetOwnerIOComponent();
	return OwnerComponent ? OwnerComponent->GetOwner() : nullptr;
}

bool UActorIOAction::IsTargetActorAlive() const
{
	if (TargetActor.IsNull())
	{
		return false;
	}

	FString ErrorReason;
	return IActorIO::ConfirmObjectIsAlive(TargetActor.Get(), ErrorReason);
}

UObject* UActorIOAction::ResolveTargetObject(const FActorIOFunction* TargetFunction) const
{
	UObject* OutTarget = nullptr;

	AActor* TargetActorPtr = TargetActor.Get();
	if (TargetActorPtr)
	{
		OutTarget = TargetActorPtr;

		// Figure out which I/O function is called by this action if not provided already.
		if (!TargetFunction)
		{
			FActorIOFunctionList ValidFunctions = IActorIO::GetFunctionsForObject(TargetActorPtr);
			TargetFunction = ValidFunctions.GetFunction(FunctionId);
		}

		// Check if the I/O function wants to be executed on a subobject instead of the target actor.
		if (TargetFunction && !TargetFunction->TargetSubobject.IsNone())
		{
			OutTarget = TargetActorPtr->GetDefaultSubobjectByName(TargetFunction->TargetSubobject);
		}
	}

	return OutTarget;
}

UFunction* UActorIOAction::ResolveUFunction(const FActorIOFunction* TargetFunction, UObject* TargetObject) const
{
	UFunction* OutFunctionPtr = nullptr;

	// Figure out which object the action is targeting if not provided already.
	if (!TargetObject)
	{
		TargetObject = ResolveTargetObject(TargetFunction);
	}

	if (TargetObject)
	{
		// Figure out which I/O function is called by this action if not provided already.
		if (!TargetFunction)
		{
			FActorIOFunctionList ValidFunctions = IActorIO::GetFunctionsForObject(TargetActor.Get());
			TargetFunction = ValidFunctions.GetFunction(FunctionId);
		}

		if (TargetFunction)
		{
			const FName FuncName = FName(*TargetFunction->FunctionToExec, FNAME_Find);
			if (FuncName != NAME_None)
			{
				OutFunctionPtr = TargetObject->GetClass()->FindFunctionByName(FuncName);
			}
		}
	}

	return OutFunctionPtr;
}

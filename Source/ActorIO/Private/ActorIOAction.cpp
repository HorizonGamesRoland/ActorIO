// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO

#include "ActorIOAction.h"
#include "ActorIOComponent.h"
#include "ActorIOInterface.h"
#include "ActorIOSubsystemBase.h"

FName UActorIOAction::ExecuteActionSignalName(TEXT("ReceiveExecuteAction"));

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

	if (!IsValid(TargetEvent->DelegateOwner))
	{
		UE_CLOG(DebugIOActions, LogActorIO, Error, TEXT("Actor '%s' could not bind action to '%s' - Delegate owner was invalid (destroyed?)."), *ActionOwner->GetActorNameOrLabel(), *EventId.ToString());
		return;
	}

	ActionDelegate = FScriptDelegate();
	ActionDelegate.BindUFunction(this, ExecuteActionSignalName);

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
			FSparseDelegate* SparseDelegate = FSparseDelegateStorage::ResolveSparseDelegate(TargetEvent->DelegateOwner, TargetEvent->SparseDelegateName);
			if (SparseDelegate)
			{
				SparseDelegate->__Internal_AddUnique(TargetEvent->DelegateOwner, TargetEvent->SparseDelegateName, ActionDelegate);
				bIsBound = true;
			}

			UE_CLOG(DebugIOActions && !bIsBound, LogActorIO, Error, TEXT("Actor '%s' could not bind action to '%s' - Failed to resolve sparse delegate with name '%s'."), *ActionOwner->GetActorNameOrLabel(), *EventId.ToString(), *TargetEvent->SparseDelegateName.ToString());
			break;
		}
		
		// Binding to blueprint delegate.
		// These are the event dispatchers created in blueprints.
		// Each event dispatcher is basically just an FMulticastDelegateProperty that we can add to.
		case FActorIOEvent::Type::BlueprintDelegate:
		{
			UClass* DelegateOwnerClass = TargetEvent->DelegateOwner->GetClass();
			FMulticastDelegateProperty* DelegateProp = CastField<FMulticastDelegateProperty>(DelegateOwnerClass->FindPropertyByName(TargetEvent->BlueprintDelegateName));
			if (DelegateProp)
			{
				DelegateProp->AddDelegate(ActionDelegate, TargetEvent->DelegateOwner);
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

	if (!TargetEvent->DelegateOwner)
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
			FSparseDelegate* SparseDelegate = FSparseDelegateStorage::ResolveSparseDelegate(TargetEvent->DelegateOwner, TargetEvent->SparseDelegateName);
			if (SparseDelegate)
			{
				SparseDelegate->__Internal_Remove(TargetEvent->DelegateOwner, TargetEvent->SparseDelegateName, ActionDelegate);
				bIsBound = false;
			}

			UE_CLOG(DebugIOActions && bIsBound, LogActorIO, Error, TEXT("Actor '%s' could not unbind action from '%s' - Failed to resolve sparse delegate with name '%s'."), *ActionOwner->GetActorNameOrLabel(), *EventId.ToString(), *TargetEvent->SparseDelegateName.ToString());
			break;
		}

		case FActorIOEvent::Type::BlueprintDelegate:
		{
			UClass* DelegateOwnerClass = TargetEvent->DelegateOwner->GetClass();
			FMulticastDelegateProperty* DelegateProp = CastField<FMulticastDelegateProperty>(DelegateOwnerClass->FindPropertyByName(TargetEvent->BlueprintDelegateName));
			if (DelegateProp)
			{
				DelegateProp->RemoveDelegate(ActionDelegate, TargetEvent->DelegateOwner);
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
	// This function is called whenever Unreal Script is executing a function on the object.
	// We are going to use this to catch when 'execute action' was called by the I/O event delegate that we are bound to.
	// The actual function bound to the delegate is empty as it's just used as an event signal here.
	// Since we cannot know what parameters the I/O event delegate has, we will preserve the params memory here.
	// Then we are going to execute the action manually so that we can have full control.

	if (Function && Function->GetFName() == ExecuteActionSignalName)
	{
		FActionExecutionContext& ExecContext = FActionExecutionContext::Get(this);
		ExecContext.EnterContext(this, Parms);

		// Start executing the action.
		// Build the command and send it to the target actor.
		ExecuteAction(ExecContext);

		// Make sure to leave the context in case 'ExecuteAction' didn't leave already.
		if (ExecContext.HasContext())
		{
			ExecContext.ExitContext();
		}
	}
	
	Super::ProcessEvent(Function, Parms);
}

void UActorIOAction::ReceiveExecuteAction()
{
	// Empty on purpose.
	// This function is only used as an event signal.
	// For more information see 'ProcessEvent' above.
}

void UActorIOAction::ExecuteAction(FActionExecutionContext& ExecutionContext)
{
	AActor* ActionOwner = GetOwnerActor();
	if (!IsValid(ActionOwner))
	{
		// Do not attempt to execute an action if we are about to be destroyed.
		return;
	}

	UE_CLOG(DebugIOActions, LogActorIO, Log, TEXT("Executing action: %s -> %s (Caller: '%s')"), *EventId.ToString(), *FunctionId.ToString(), *ActionOwner->GetActorNameOrLabel());

	if (!IsValid(TargetActor))
	{
		// Do nothing if the target actor is invalid.
		// The actor was most likely destroyed at runtime.
		UE_CLOG(DebugIOActions && WarnIOInvalidTarget, LogActorIO, Warning, TEXT("Could not find target actor. Actor was destroyed?"));
		return;
	}

	FActorIOFunctionList ValidFunctions = IActorIO::GetFunctionsForObject(TargetActor);
	FActorIOFunction* TargetFunction = ValidFunctions.GetFunction(FunctionId);
	if (!TargetFunction)
	{
		UE_CLOG(DebugIOActions, LogActorIO, Error, TEXT("Could not find function '%s' on target actor '%s'."), *FunctionId.ToString(), *TargetActor->GetActorNameOrLabel());
		return;
	}

	if (TargetFunction->FunctionToExec.IsEmpty())
	{
		UE_CLOG(DebugIOActions, LogActorIO, Error, TEXT("Function '%s' points to an empty func name."), *FunctionId.ToString());
		return;
	}

	// Figure out the object that the final command will be sent to.
	// The I/O function may want it to be executed on a subobject rather then the target actor itself.
	UObject* ObjectToSendCommandTo = TargetActor;
	if (!TargetFunction->TargetSubobject.IsNone())
	{
		ObjectToSendCommandTo = TargetActor->GetDefaultSubobjectByName(TargetFunction->TargetSubobject);
		if (!IsValid(ObjectToSendCommandTo))
		{
			UE_CLOG(DebugIOActions, LogActorIO, Error, TEXT("Could not find default subobject '%s' on target actor '%s'."), *TargetFunction->TargetSubobject.ToString(), *TargetActor->GetActorNameOrLabel());
			return;
		}
	}

	const bool bProcessNamedArgs = FunctionArguments.Contains(NAMEDARGUMENT_PREFIX);
	if (bProcessNamedArgs)
	{
		// Let the I/O subsystem to add globally available named arguments to the current execution context.
		// Think stuff like reference to player character, or player controller.
		UActorIOSubsystemBase* IOSubsystem = UActorIOSubsystemBase::Get(this);
		IOSubsystem->GetGlobalNamedArguments(ExecutionContext);

		// Let the event processor assign values to arbitrary named arguments.
		// We are calling the event processor with the original params memory that we received from the delegate.
		// This way the event processor will receive the proper values for its params given that its signature matches the delegate.
		FActorIOEventList ValidEvents = IActorIO::GetEventsForObject(ActionOwner);
		FActorIOEvent* BoundEvent = ValidEvents.GetEvent(EventId);
		check(BoundEvent);
		if (BoundEvent->EventProcessor.IsBound())
		{
			UObject* EventProcessorObject = BoundEvent->EventProcessor.GetUObject();
			if (IsValid(EventProcessorObject))
			{
				UFunction* Func_EventProcessor = EventProcessorObject->GetClass()->FindFunctionByName(BoundEvent->EventProcessor.GetFunctionName());
				EventProcessorObject->ProcessEvent(Func_EventProcessor, ExecutionContext.ScriptParams);
			}
		}
	}

	// Log named arguments to console for debugging if needed.
	if (LogIONamedArgs)
	{
		UE_LOG(LogActorIO, Log, TEXT("  Named Arguments: (%d)"), ExecutionContext.NamedArguments.Num());
		if (bProcessNamedArgs)
		{
			for (const TPair<FString, FString>& NamedArg : ExecutionContext.NamedArguments)
			{
				UE_LOG(LogActorIO, Log, TEXT("  - %s = %s"), *NamedArg.Key, *NamedArg.Value);
			}
		}
		else
		{
			UE_LOG(LogActorIO, Log, TEXT("  - Skipped because parameters field didn't contain any named args."));
		}
	}

	// Give the owning actor a chance to abort action execution.
	// Deliberately doing this after collecting named arguments in case we want to access them.
	if (ActionOwner->Implements<UActorIOInterface>())
	{
		if (IActorIOInterface::Execute_ConditionalAbortIOAction(ActionOwner, this))
		{
			UE_CLOG(DebugIOActions, LogActorIO, Log, TEXT("Action aborted by actor."));
			return;
		}
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

	// Get the quoted name of the function to call on the target actor.
	// Quotes are needed to support function names with whitespaces.
	FString FunctionName = TargetFunction->FunctionToExec;
	if (FunctionName.Len() > 0)
	{
		if (FunctionName[0] != '"')
		{
			FunctionName.InsertAt(0, '"');
		}

		const int32 LastCharIndex = FunctionName.Len() - 1;
		if (FunctionName[LastCharIndex] != '"')
		{
			FunctionName.AppendChar('"');
		}
	}

	// Build the final command that is sent to the target actor.
	// Format is: FunctionName Arg1 Arg2 Arg3 (...)
	FString Command = FunctionName;
	for (const FString& Argument : Arguments)
	{
		Command.Append(TEXT(" "));
		Command.Append(Argument);
	}

	UE_CLOG(DebugIOActions && LogIOFinalCommand, LogActorIO, Log, TEXT("  Final command sent : %s"), *Command);

	ExecutionContext.ExitContext();
	bWasExecuted = true;

	// Send the final command to the target actor.
	if (Delay > 0.0f)
	{
		FTimerHandle UniqueHandle;
		FTimerDelegate SendCommandDelegate = FTimerDelegate::CreateUObject(this, &ThisClass::SendCommand, ObjectToSendCommandTo, Command);
		GetWorld()->GetTimerManager().SetTimer(UniqueHandle, SendCommandDelegate, Delay, false);
	}
	else
	{
		SendCommand(ObjectToSendCommandTo, Command);
	}
}

void UActorIOAction::SendCommand(UObject* TargetObject, FString Command)
{
	if (IsValid(TargetObject))
	{
		FStringOutputDevice Ar;
		TargetObject->CallFunctionByNameWithArguments(*Command, Ar, this, true);

		// Log execution errors.
		if (!Ar.IsEmpty())
		{
			UE_CLOG(DebugIOActions, LogActorIO, Error, TEXT("%s - Original command: '%s'"), *Ar, *Command);
		}
	}
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

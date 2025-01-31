// Copyright 2025 Horizon Games. All Rights Reserved.

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
		UE_LOG(LogActorIO, Error, TEXT("Actor '%s' could not bind action to '%s' - Action is already bound!"), *ActionOwner->GetActorNameOrLabel(), *EventId.ToString());
		return;
	}

	const FActorIOEventList ValidEvents = IActorIO::GetEventsForObject(ActionOwner);
	const FActorIOEvent* TargetEvent = ValidEvents.GetEvent(EventId);
	if (!TargetEvent)
	{
		UE_LOG(LogActorIO, Error, TEXT("Actor '%s' could not bind action to '%s' - Event was not found."), *ActionOwner->GetActorNameOrLabel(), *EventId.ToString());
		return;
	}

	if (!IsValid(TargetEvent->DelegateOwner))
	{
		UE_LOG(LogActorIO, Error, TEXT("Actor '%s' could not bind action to '%s' - Delegate owner was invalid (destroyed?)."), *ActionOwner->GetActorNameOrLabel(), *EventId.ToString());
		return;
	}

	ActionDelegate = FScriptDelegate();
	ActionDelegate.BindUFunction(this, ExecuteActionSignalName);

	// Binding to multicast delegate directly:
	// Since we have a direct reference to the delegate, we can simply add to it.
	if (TargetEvent->MulticastDelegatePtr)
	{
		TargetEvent->MulticastDelegatePtr->Add(ActionDelegate);
		bIsBound = true;
	}

	// Binding to a sparse delegate.
	// These delegates are stored in a global storage so we need to resolve it first.
	// Then we have to use the interal add function because there's no other way to set the bIsBound param for it.
	else if (!TargetEvent->SparseDelegateName.IsNone())
	{
		FSparseDelegate* SparseDelegate = FSparseDelegateStorage::ResolveSparseDelegate(TargetEvent->DelegateOwner, TargetEvent->SparseDelegateName);
		if (SparseDelegate)
		{
			SparseDelegate->__Internal_AddUnique(TargetEvent->DelegateOwner, TargetEvent->SparseDelegateName, ActionDelegate);
			bIsBound = true;
		}
		
		UE_CLOG(!bIsBound, LogActorIO, Error, TEXT("Actor '%s' could not bind action to '%s' - Failed to resolve sparse delegate with name '%s'."),
			*ActionOwner->GetActorNameOrLabel(), *EventId.ToString(), *TargetEvent->SparseDelegateName.ToString());
	}

	// Binding to blueprint delegate.
	// These are the event dispatchers created in blueprints.
	// Each event dispatcher is basically just an FMulticastDelegateProperty that we can add to.
	else if (!TargetEvent->BlueprintDelegateName.IsNone())
	{
		UClass* DelegateOwnerClass = TargetEvent->DelegateOwner->GetClass();
		FMulticastDelegateProperty* DelegateProp = CastField<FMulticastDelegateProperty>(DelegateOwnerClass->FindPropertyByName(TargetEvent->BlueprintDelegateName));
		if (DelegateProp)
		{
			DelegateProp->AddDelegate(ActionDelegate, TargetEvent->DelegateOwner);
			bIsBound = true;
		}

		UE_CLOG(!bIsBound, LogActorIO, Error, TEXT("Actor '%s' could not bind action to '%s' - No event dispatcher found with name '%s'."),
			*ActionOwner->GetActorNameOrLabel(), *EventId.ToString(), *TargetEvent->BlueprintDelegateName.ToString());
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
		checkf(TargetEvent, TEXT("Could not unbind action because the I/O event that we were bound to was not found?!"));
	}

	// Unbinding from multicast delegate directly.
	FMulticastScriptDelegate* TargetDelegate = TargetEvent->MulticastDelegatePtr;
	if (TargetDelegate)
	{
		TargetDelegate->Remove(ActionDelegate);
		bIsBound = false;
	}

	// Unbinding from a sparse delegate.
	else if (!TargetEvent->SparseDelegateName.IsNone())
	{
		FSparseDelegate* SparseDelegate = FSparseDelegateStorage::ResolveSparseDelegate(TargetEvent->DelegateOwner, TargetEvent->SparseDelegateName);
		if (SparseDelegate)
		{
			SparseDelegate->__Internal_Remove(TargetEvent->DelegateOwner, TargetEvent->SparseDelegateName, ActionDelegate);
			bIsBound = false;
		}

		UE_CLOG(bIsBound, LogActorIO, Error, TEXT("Actor '%s' could not unbind action from '%s' - Failed to resolve sparse delegate with name '%s'."),
			*ActionOwner->GetActorNameOrLabel(), *EventId.ToString(), *TargetEvent->SparseDelegateName.ToString());
	}

	// Unbinding from a blueprint delegate.
	else if (!TargetEvent->BlueprintDelegateName.IsNone())
	{
		UClass* DelegateOwnerClass = TargetEvent->DelegateOwner->GetClass();
		FMulticastDelegateProperty* DelegateProp = CastField<FMulticastDelegateProperty>(DelegateOwnerClass->FindPropertyByName(TargetEvent->BlueprintDelegateName));
		if (DelegateProp)
		{
			DelegateProp->RemoveDelegate(ActionDelegate, TargetEvent->DelegateOwner);
			bIsBound = false;
		}

		UE_CLOG(bIsBound, LogActorIO, Error, TEXT("Actor '%s' could not unbind action from '%s' - No event dispatcher found with name '%s'."),
			*ActionOwner->GetActorNameOrLabel(), *EventId.ToString(), *TargetEvent->BlueprintDelegateName.ToString());
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

	UE_LOG(LogActorIO, Log, TEXT("Actor '%s' executing action: %s -> %s"), *ActionOwner->GetActorNameOrLabel(), *EventId.ToString(), *FunctionId.ToString());

	if (!IsValid(TargetActor))
	{
		// Do nothing if the target actor is invalid.
		// The actor was most likely destroyed at runtime.
		UE_LOG(LogActorIO, Warning, TEXT("Actor '%s' failed to execute action. Could not find target actor '%s'. Actor was destroyed?"),
			*ActionOwner->GetActorNameOrLabel(), *TargetActor->GetActorNameOrLabel());
		return;
	}

	FActorIOFunctionList ValidFunctions = IActorIO::GetFunctionsForObject(TargetActor);
	FActorIOFunction* TargetFunction = ValidFunctions.GetFunction(FunctionId);
	if (!TargetFunction)
	{
		UE_LOG(LogActorIO, Error, TEXT("Actor '%s' failed to execute action. Could not find function '%s' on target actor '%s'."),
			*ActionOwner->GetActorNameOrLabel(), *FunctionId.ToString(), *TargetActor->GetActorNameOrLabel());
		return;
	}

	if (TargetFunction->FunctionToExec.IsEmpty())
	{
		UE_LOG(LogActorIO, Error, TEXT("Actor '%s' failed to execute action. Function '%s' points to an empty func name."),
			*ActionOwner->GetActorNameOrLabel(), *FunctionId.ToString());
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
			UE_LOG(LogActorIO, Error, TEXT("Actor '%s' failed to execute action. Could not find default subobject '%s' on target actor '%s'."),
				*ActionOwner->GetActorNameOrLabel(), *TargetFunction->TargetSubobject.ToString(), *TargetActor->GetActorNameOrLabel());
			return;
		}
	}

	if (FunctionArguments.Contains(NAMEDARGUMENT_PREFIX))
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

	// Give the owning actor a chance to abort action execution.
	// Deliberately doing this after collecting named arguments in case we want to access them.
	if (ActionOwner->Implements<UActorIOInterface>())
	{
		if (IActorIOInterface::Execute_ConditionalAbortIOAction(ActionOwner, this))
		{
			UE_LOG(LogActorIO, Log, TEXT("Actor '%s' aborted action execution."), *ActionOwner->GetActorNameOrLabel());
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
			UE_LOG(LogActorIO, Error, TEXT("%s - Original command: '%s'"), *Ar, *Command);
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

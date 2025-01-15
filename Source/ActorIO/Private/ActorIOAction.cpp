// Copyright 2025 Horizon Games. All Rights Reserved.

#include "ActorIOAction.h"
#include "ActorIOSystem.h"
#include "ActorIOComponent.h"
#include "ActorIOInterface.h"
#include "UObject/SparseDelegate.h"
#include "Misc/OutputDeviceNull.h"

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
	// #TODO: Rework error logs

	if (bIsBound)
	{
		UE_LOG(LogActorIO, Error, TEXT("Attempted to bind an action that was bound already!"));
		return;
	}

	const FActorIOEventList ValidEvents = UActorIOSystem::GetEventsForObject(GetOwnerActor());
	const FActorIOEvent* TargetEvent = ValidEvents.FindByKey(EventId);
	if (!TargetEvent)
	{
		UE_LOG(LogActorIO, Error, TEXT("Could not find I/O event named '%s' on actor '%s'"), *EventId.ToString(), *GetOwnerActor()->GetActorNameOrLabel());
		return;
	}

	if (!IsValid(TargetEvent->DelegateOwner))
	{
		return;
	}

	ActionDelegate = FScriptDelegate();
	ActionDelegate.BindUFunction(this, ExecuteActionSignalName);

	if (TargetEvent->MulticastDelegatePtr)
	{
		TargetEvent->MulticastDelegatePtr->Add(ActionDelegate);
		bIsBound = true;
	}
	else if (!TargetEvent->SparseDelegateName.IsNone())
	{
		FSparseDelegate* SparseDelegate = FSparseDelegateStorage::ResolveSparseDelegate(TargetEvent->DelegateOwner, TargetEvent->SparseDelegateName);
		if (SparseDelegate)
		{
			SparseDelegate->__Internal_AddUnique(TargetEvent->DelegateOwner, TargetEvent->SparseDelegateName, ActionDelegate);
			bIsBound = true;
		}
	}
	else if (!TargetEvent->BlueprintDelegateName.IsNone())
	{
		UClass* DelegateOwnerClass = TargetEvent->DelegateOwner->GetClass();
		FMulticastDelegateProperty* DelegateProp = CastField<FMulticastDelegateProperty>(DelegateOwnerClass->FindPropertyByName(TargetEvent->BlueprintDelegateName));
		if (DelegateProp)
		{
			DelegateProp->AddDelegate(ActionDelegate, TargetEvent->DelegateOwner);
			bIsBound = true;
		}
	}

	if (!bIsBound)
	{
		UE_LOG(LogActorIO, Error, TEXT("Could not bind action to '%s'"), *TargetEvent->EventId.ToString());
	}
}

void UActorIOAction::UnbindAction()
{
	if (!bIsBound)
	{
		return;
	}

	UActorIOComponent* OwnerIOComponent = GetOwnerIOComponent();
	if (!OwnerIOComponent)
	{
		return;
	}

	const FActorIOEventList ValidEvents = UActorIOSystem::GetEventsForObject(OwnerIOComponent->GetOwner());
	const FActorIOEvent* TargetEvent = ValidEvents.FindByKey(EventId);
	if (!TargetEvent)
	{
		return;
	}

	FMulticastScriptDelegate* TargetDelegate = TargetEvent->MulticastDelegatePtr;
	if (TargetDelegate)
	{
		TargetDelegate->Remove(ActionDelegate);
		bIsBound = false;
	}
	else if (!TargetEvent->SparseDelegateName.IsNone())
	{
		FSparseDelegate* SparseDelegate = FSparseDelegateStorage::ResolveSparseDelegate(TargetEvent->DelegateOwner, TargetEvent->SparseDelegateName);
		if (SparseDelegate)
		{
			SparseDelegate->__Internal_Remove(TargetEvent->DelegateOwner, TargetEvent->SparseDelegateName, ActionDelegate);
			bIsBound = false;
		}
	}
	else if (!TargetEvent->BlueprintDelegateName.IsNone())
	{
		UClass* DelegateOwnerClass = TargetEvent->DelegateOwner->GetClass();
		FMulticastDelegateProperty* DelegateProp = CastField<FMulticastDelegateProperty>(DelegateOwnerClass->FindPropertyByName(TargetEvent->BlueprintDelegateName));
		if (DelegateProp)
		{
			DelegateProp->RemoveDelegate(ActionDelegate, TargetEvent->DelegateOwner);
			bIsBound = false;
		}
	}
}

void UActorIOAction::ProcessEvent(UFunction* Function, void* Parms)
{
	if (Function->GetFName() == ExecuteActionSignalName)
	{
		FActionExecutionContext& ExecContext = FActionExecutionContext::Get(this);
		ExecContext.EnterContext(this, Parms);

		if (CanExecuteAction(ExecContext))
		{
			ExecuteAction(ExecContext);
		}

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
}

bool UActorIOAction::CanExecuteAction(FActionExecutionContext& ExecutionContext)
{
	const AActor* ActionOwner = GetOwnerActor();
	if (!IsValid(ActionOwner))
	{
		// Do not attempt to execute an action if we are about to be destroyed.
		return false;
	}

	if (bExecuteOnlyOnce && bWasExecuted)
	{
		// Action has already been executed once.
		return false;
	}

	return true;
}

void UActorIOAction::ExecuteAction(FActionExecutionContext& ExecutionContext)
{
	UE_LOG(LogActorIO, Log, TEXT("Executing action: %s -> %s"), *EventId.ToString(), *FunctionId.ToString());

	if (!IsValid(TargetActor))
	{
		// Do nothing if the target actor is invalid.
		// The actor was most likely destroyed at runtime.
		return;
	}

	FActorIOFunctionList ValidFunctions = UActorIOSystem::GetFunctionsForObject(TargetActor);
	FActorIOFunction* TargetFunction = ValidFunctions.FindByKey(FunctionId);
	if (!TargetFunction)
	{
		UE_LOG(LogActorIO, Error, TEXT("Failed to find function '%s' on target actor '%s'"), *FunctionId.ToString(), *TargetActor->GetActorNameOrLabel());
		return;
	}

	// Figure out the final object that the function will be executed on.
	// The function may want it to be executed on a subobject of the target actor.
	UObject* ObjectToSendCommandTo = TargetActor;
	if (!TargetFunction->TargetSubobject.IsNone())
	{
		ObjectToSendCommandTo = TargetActor->GetDefaultSubobjectByName(TargetFunction->TargetSubobject);
		if (!IsValid(ObjectToSendCommandTo))
		{
			UE_LOG(LogActorIO, Error, TEXT("Failed to find default subobject '%s' on target actor '%s'"), *TargetFunction->TargetSubobject.ToString(), *TargetActor->GetActorNameOrLabel());
			return;
		}
	}

	FActorIOEventList ValidEvents = UActorIOSystem::GetEventsForObject(GetOwnerActor());
	FActorIOEvent* BoundEvent = ValidEvents.FindByKey(EventId);
	check(BoundEvent);

	// Let the event processor assign values to arbitrary named arguments.
	// We are calling the event processor with the original script params memory that we received from the delegate that our action is bound to.
	// This way the event processor will receive the proper values for its params given that its signature matches the delegate.
	if (BoundEvent->EventProcessor.IsBound())
	{
		UObject* EventProcessorObject = BoundEvent->EventProcessor.GetUObject();
		if (IsValid(EventProcessorObject))
		{
			UFunction* Func_EventProcessor = EventProcessorObject->GetClass()->FindFunctionByName(BoundEvent->EventProcessor.GetFunctionName());
			EventProcessorObject->ProcessEvent(Func_EventProcessor, ExecutionContext.ScriptParams);
		}
	}

	// Break up the user defined arguments string from a single line into multiple elements.
	// Then replace all named arguments with their actual value set by the 'EventProcessor' above.
	// Everything stays in string form (including named argument values) until the very end when command is sent.
	TArray<FString> Arguments;
	if (FunctionArguments.ParseIntoArray(Arguments, TEXT(" "), true) > 0)
	{
		for (FString& Argument : Arguments)
		{
			if (Argument.StartsWith(NAMEDARGUMENT_PREFIX) && ExecutionContext.NamedArguments.Contains(Argument))
			{
				const FString NamedArgValue = ExecutionContext.NamedArguments[Argument];
				Argument = NamedArgValue;
			}
		}
	}

	// Build the final command that's sent to the target actor.
	// Format is: FunctionName Arg1 Arg2 Arg3 (...)
	FString Command = TargetFunction->FunctionToExec;
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
		FOutputDeviceNull Ar;
		TargetObject->CallFunctionByNameWithArguments(*Command, Ar, this, true);
	}
}

UActorIOComponent* UActorIOAction::GetOwnerIOComponent() const
{
	// Actions are owned by the actor's ActorIO component.
	return Cast<UActorIOComponent>(GetOuter());
}

AActor* UActorIOAction::GetOwnerActor() const
{
	UActorIOComponent* OwnerComponent = GetOwnerIOComponent();
	return OwnerComponent ? OwnerComponent->GetOwner() : nullptr;
}

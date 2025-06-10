// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOAction.h"
#include "ActorIOComponent.h"
#include "ActorIOInterface.h"
#include "ActorIOSubsystemBase.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/EngineVersionComparison.h"

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

	if (bExecuteOnlyOnce && bWasExecuted)
	{
		// Do nothing if execute only once is enabled and the action was executed already.
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
		SendCommandInternal(TargetObject, *Command, Ar, this);

		// Log execution errors.
		if (!Ar.IsEmpty())
		{
			UE_CLOG(DebugIOActions, LogActorIO, Error, TEXT("%s"), *Ar);
		}
	}
}

bool UActorIOAction::SendCommandInternal(UObject* TargetObject, const TCHAR* Str, FOutputDevice& Ar, UObject* Executor)
{
	/**
	 * THIS IS A MODIFIED VERSION OF UObject::CallFunctionByNameWithString
	 * 
	 * Always review the original function when a new engine version is released and update this code if needed.
	 * If everything is working update the UE version comparison below. Current value represents the latest reviewed engine version.
	 * 
	 * List of changes:
	 * 
	 *   - Skip CPP param default value initialization because it only works in editor and not packaged games.
	 *   - FindFunction and ProcessEvent are called on TargetObject.
	 *   - Use Ar.Logf instead of UE_LOG(LogScriptCore) because LogScriptCore is static and its verbosity cannot be changed.
	 */

#if UE_VERSION_NEWER_THAN(5, 6, 999) // <- patch version doesn't matter so use 999 to pass the check
#error "Review latest implementation of UObject::CallFunctionByNameWithString then update UE version comparison."
#endif

	// Find an exec function.
	FString MsgStr;
	if (!FParse::Token(Str, MsgStr, true))
	{
		Ar.Logf(TEXT("CallFunctionByNameWithArguments: Not Parsed '%s'"), Str);
		return false;
	}
	const FName Message = FName(*MsgStr, FNAME_Find);
	if (Message == NAME_None)
	{
		Ar.Logf(TEXT("CallFunctionByNameWithArguments: Name not found '%s'"), Str);
		return false;
	}
	UFunction* Function = TargetObject->FindFunction(Message);
	if (nullptr == Function)
	{
		Ar.Logf(TEXT("CallFunctionByNameWithArguments: Function not found '%s'"), Str);
		return false;
	}

	FProperty* LastParameter = nullptr;

	// find the last parameter
	for (TFieldIterator<FProperty> It(Function); It && (It->PropertyFlags & (CPF_Parm | CPF_ReturnParm)) == CPF_Parm; ++It)
	{
		LastParameter = *It;
	}

	// Parse all function parameters.
	uint8* Parms = (uint8*)FMemory_Alloca_Aligned(Function->ParmsSize, Function->GetMinAlignment());
	FMemory::Memzero(Parms, Function->ParmsSize);

	for (TFieldIterator<FProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		FProperty* LocalProp = *It;
		checkSlow(LocalProp);
		if (!LocalProp->HasAnyPropertyFlags(CPF_ZeroConstructor))
		{
			LocalProp->InitializeValue_InContainer(Parms);
		}
	}

	const uint32 ExportFlags = PPF_None;
	bool bFailed = 0;
	int32 NumParamsEvaluated = 0;
	for (TFieldIterator<FProperty> It(Function); It && (It->PropertyFlags & (CPF_Parm | CPF_ReturnParm)) == CPF_Parm; ++It, NumParamsEvaluated++)
	{
		FProperty* PropertyParam = *It;
		checkSlow(PropertyParam); // Fix static analysis warning
		if (NumParamsEvaluated == 0 && Executor)
		{
			FObjectPropertyBase* Op = CastField<FObjectPropertyBase>(*It);
			if (Op && Executor->IsA(Op->PropertyClass))
			{
				// First parameter is implicit reference to object executing the command.
				Op->SetObjectPropertyValue(Op->ContainerPtrToValuePtr<uint8>(Parms), Executor);
				continue;
			}
		}

		// Keep old string around in case we need to pass the whole remaining string
		const TCHAR* RemainingStr = Str;

		// Parse a new argument out of Str
		FString ArgStr;
		FParse::Token(Str, ArgStr, true);

		// if ArgStr is empty but we have more params to read parse the function to see if these have defaults, if so set them
		bool bFoundDefault = false;
		bool bFailedImport = true;

		/*
		 * SKIP INITIALIZE CPP FUNCTION PARAM DEFAULT VALUE
		 * 
		 * This only works in the editor because values are being read from Function->GetMetaData (which is editor only).
		 * We need to skip this because it would lead to different outcomes in editor vs packaged game.
		 * I'm not sure how blueprint nodes do this.. I suspect they read this value at edit time and store it for execution.
		 * In theory we could cache these values at cook time for use at runtime (?)
		 *
#if WITH_EDITOR
		if (!FCString::Strcmp(*ArgStr, TEXT("")))
		{
			const FName DefaultPropertyKey(*(FString(TEXT("CPP_Default_")) + PropertyParam->GetName()));
			const FString& PropertyDefaultValue = Function->GetMetaData(DefaultPropertyKey);
			if (!PropertyDefaultValue.IsEmpty())
			{
				bFoundDefault = true;

				const TCHAR* Result = It->ImportText_InContainer(*PropertyDefaultValue, Parms, nullptr, ExportFlags);
				bFailedImport = (Result == nullptr);
			}
		}
#endif
		*/ 

		if (!bFoundDefault)
		{
			// if this is the last string property and we have remaining arguments to process, we have to assume that this
			// is a sub-command that will be passed to another exec (like "cheat giveall weapons", for example). Therefore
			// we need to use the whole remaining string as an argument, regardless of quotes, spaces etc.
			if (PropertyParam == LastParameter && PropertyParam->IsA<FStrProperty>() && FCString::Strcmp(Str, TEXT("")) != 0)
			{
				ArgStr = FString(RemainingStr).TrimStart();
			}

			const TCHAR* Result = It->ImportText_InContainer(*ArgStr, Parms, nullptr, ExportFlags);
			bFailedImport = (Result == nullptr);
		}

		if (bFailedImport)
		{
			FFormatNamedArguments Arguments;
			Arguments.Add(TEXT("Message"), FText::FromName(Message));
			Arguments.Add(TEXT("PropertyName"), FText::FromName(It->GetFName()));
			Arguments.Add(TEXT("FunctionName"), FText::FromName(Function->GetFName()));
			Ar.Logf(TEXT("%s"), *FText::Format(NSLOCTEXT("Core", "BadProperty", "'{Message}': Bad or missing property '{PropertyName}' when trying to call {FunctionName}"), Arguments).ToString());
			bFailed = true;

			break;
		}
	}

	if (!bFailed)
	{
		TargetObject->ProcessEvent(Function, Parms);
	}

	//!!destructframe see also UObject::ProcessEvent
	for (TFieldIterator<FProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		It->DestroyValue_InContainer(Parms);
	}

	// Success.
	return true;
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

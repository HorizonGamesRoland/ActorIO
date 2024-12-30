// Copyright 2024 Horizon Games. All Rights Reserved.

#include "ActorIOAction.h"
#include "ActorIOEvent.h"
#include "ActorIOFunction.h"
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
	if (bIsBound)
	{
		return;
	}

	UActorIOComponent* OwnerIOComponent = GetOwnerIOComponent();
	if (!OwnerIOComponent)
	{
		return;
	}

	const TArray<FActorIOEvent> ValidEvents = UActorIOSystem::GetEventsForObject(OwnerIOComponent->GetOwner());
	const FActorIOEvent* TargetEvent = ValidEvents.FindByKey(EventId);
	if (!TargetEvent)
	{
		return;
	}

	if (!IsValid(TargetEvent->DelegateOwner))
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Binding an action to: %s"), *TargetEvent->EventId.ToString());

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
		UE_LOG(LogTemp, Error, TEXT("Could not bind action to '%s'"), *TargetEvent->EventId.ToString());
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

	const TArray<FActorIOEvent> ValidEvents = UActorIOSystem::GetEventsForObject(OwnerIOComponent->GetOwner());
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
			ProcessAction(ExecContext);
		}

		if (ExecContext.HasContext())
		{
			ExecContext.LeaveContext();
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
	if (!IsValid(ActionOwner) || !IsValid(TargetActor))
	{
		// Do not attempt to execute action on an invalid actor.
		return false;
	}

	if (bExecuteOnlyOnce && bWasExecuted)
	{
		// Action has already been executed once.
		return false;
	}

	return true;
}

void UActorIOAction::ProcessAction(FActionExecutionContext& ExecutionContext)
{
	AActor* ActionOwner = GetOwnerActor();
	TArray<FActorIOEvent> ValidEvents = UActorIOSystem::GetEventsForObject(ActionOwner);
	TArray<FActorIOFunction> ValidFunctions = UActorIOSystem::GetFunctionsForObject(TargetActor);

	FActorIOFunction* TargetFunction = ValidFunctions.FindByKey(FunctionId);
	if (!TargetFunction)
	{
		UE_LOG(LogTemp, Error, TEXT("ActorIOAction: Function '%s' was not found on target actor '%s'"), *FunctionId.ToString(), *TargetActor->GetActorNameOrLabel());
		return;
	}

	FActorIOEvent* BoundEvent = ValidEvents.FindByKey(EventId);
	if (BoundEvent->EventProcessor.IsBound())
	{
		UObject* EventProcessorObject = BoundEvent->EventProcessor.GetUObject();
		if (IsValid(EventProcessorObject))
		{
			UFunction* Func_EventProcessor = EventProcessorObject->GetClass()->FindFunctionByName(BoundEvent->EventProcessor.GetFunctionName());
			EventProcessorObject->ProcessEvent(Func_EventProcessor, ExecutionContext.ScriptParams);
		}
	}

	FString Arguments = FunctionArguments;
	for (const TPair<FString, FString>& NamedArgument : ExecutionContext.NamedArguments)
	{
		// #TODO: Parse arguments in form of $NamedArgument?
	}

	FString Command = TargetFunction->FunctionToExec;
	if (!Arguments.IsEmpty())
	{
		Command.Append(TEXT(" "));
		Command.Append(Arguments);
	}

	ExecutionContext.LeaveContext();
	bWasExecuted = true;

	if (Delay > 0.0f)
	{
		FTimerHandle UniqueHandle;
		FTimerDelegate ExecuteActionDelegate = FTimerDelegate::CreateUObject(this, &ThisClass::ExecuteAction, Command);
		GetWorld()->GetTimerManager().SetTimer(UniqueHandle, ExecuteActionDelegate, Delay, false);
	}
	else
	{
		ExecuteAction(Command);
	}
}

void UActorIOAction::ExecuteAction(FString Command)
{
	UE_LOG(LogTemp, Warning, TEXT("Executing action: %s"), *Command);

	FOutputDeviceNull Ar;
	TargetActor->CallFunctionByNameWithArguments(*Command, Ar, this, true);
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

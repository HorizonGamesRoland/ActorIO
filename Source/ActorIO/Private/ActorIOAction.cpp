// Copyright 2024 Horizon Games. All Rights Reserved.

#include "ActorIOAction.h"
#include "ActorIOEvent.h"
#include "ActorIOFunction.h"
#include "ActorIOSystem.h"
#include "ActorIOComponent.h"
#include "ActorIOInterface.h"
#include "UObject/SparseDelegate.h"
#include "Misc/OutputDeviceNull.h"

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
	ActionDelegate.BindUFunction(this, TEXT("OnEventDelegateTriggered"));

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
	if (Function->GetFName() == TEXT("OnEventDelegateTriggered"))
	{
		FActionExecutionContext& ExecContext = FActionExecutionContext::Get(this);
		ExecContext.EnterContext(this, Parms);

		ExecuteAction(ExecContext);
	}
	else
	{
		Super::ProcessEvent(Function, Parms);
	}
}

void UActorIOAction::OnEventDelegateTriggered()
{
	checkNoEntry();
}

void UActorIOAction::ExecuteAction(FActionExecutionContext& ExecutionContext)
{
	AActor* ActionOwner = GetOwnerActor();
 	if (!IsValid(ActionOwner) || !IsValid(TargetActor))
	{
		// Do not attempt to execute action on an invalid actor.
		return;
	}

	if (bExecuteOnlyOnce && bWasExecuted)
	{
		// Action has already been executed once.
		return;
	}

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
		UFunction* Func_EventProcessor = BoundEvent->EventProcessor.GetUObject()->GetClass()->FindFunctionByName(BoundEvent->EventProcessor.GetFunctionName());
		BoundEvent->EventProcessor.GetUObject()->ProcessEvent(Func_EventProcessor, ExecutionContext.ScriptParams);
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

	UE_LOG(LogTemp, Warning, TEXT("Executing action: %s"), *Command);

	FTimerDelegate ExecutionDelegate = FTimerDelegate::CreateLambda([this, Command]()
	{
		FOutputDeviceNull Ar;
		TargetActor->CallFunctionByNameWithArguments(*Command, Ar, this, true);
	});

	ExecutionContext.LeaveContext();

	if (Delay > 0.0f)
	{
		FTimerHandle UnusedTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(UnusedTimerHandle, ExecutionDelegate, Delay, false);
	}
	else
	{
		ExecutionDelegate.Execute();
	}

	bWasExecuted = true;
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

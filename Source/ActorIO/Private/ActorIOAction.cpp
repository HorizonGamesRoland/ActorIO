// Copyright 2024 Horizon Games. All Rights Reserved.

#include "ActorIOAction.h"
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

	bWasExecuted = false;
	bIsBound = false;
	ActionDelegate = FScriptDelegate();
}

void UActorIOAction::BindAction()
{
	UActorIOComponent* OwnerIOComponent = GetOwnerIOComponent();
	if (!OwnerIOComponent)
	{
		return;
	}

	const TArray<FActorIOEvent> ValidEvents = UActorIOComponent::GetEventsForObject(OwnerIOComponent->GetOwner());
	const FActorIOEvent* TargetEvent = ValidEvents.FindByKey(EventId);
	if (!TargetEvent)
	{
		return;
	}

	if (!IsValid(TargetEvent->DelegateOwner))
	{
		return;
	}

	if (bIsBound)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Binding an action to: %s"), *TargetEvent->EventId.ToString());

	ActionDelegate = FScriptDelegate();
	ActionDelegate.BindUFunction(this, TEXT("ExecuteAction"));

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
	UActorIOComponent* OwnerIOComponent = GetOwnerIOComponent();
	if (!OwnerIOComponent)
	{
		return;
	}

	const TArray<FActorIOEvent> ValidEvents = UActorIOComponent::GetEventsForObject(OwnerIOComponent->GetOwner());
	const FActorIOEvent* TargetEvent = ValidEvents.FindByKey(EventId);
	if (!TargetEvent)
	{
		return;
	}

	if (bIsBound)
	{
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
}

void UActorIOAction::ExecuteAction()
{
 	if (!IsValid(TargetActor))
	{
		// The target actor was invalid.
		// Actor was most likely destroyed.
		return;
	}

	TArray<FActorIOFunction> ValidFunctions = UActorIOComponent::GetFunctionsForObject(TargetActor);
	FActorIOFunction* TargetFunction = ValidFunctions.FindByKey(FunctionId);
	if (!TargetFunction)
	{
		UE_LOG(LogTemp, Error, TEXT("ActorIOLink: Function '%s' was not found on target actor '%s'"), *FunctionId.ToString(), *TargetActor->GetActorNameOrLabel());
		return;
	}

	FString Command = TargetFunction->FunctionToExec;
	Command.Append(TEXT(" "));
	Command.Append(FunctionArguments);

	UE_LOG(LogTemp, Warning, TEXT("Executing action: %s"), *Command);

	FOutputDeviceNull Ar;
	if (TargetActor->CallFunctionByNameWithArguments(*Command, Ar, this, true))
	{
		bWasExecuted = true;
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

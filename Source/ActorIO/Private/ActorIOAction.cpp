// Copyright 2024 Horizon Games. All Rights Reserved.

#include "ActorIOAction.h"
#include "ActorIOComponent.h"
#include "ActorIOInterface.h"
#include "UObject/SparseDelegate.h"
#include "Misc/OutputDeviceNull.h"

UActorIOLink::UActorIOLink()
{
	LinkedAction = FActorIOAction();
	bWasExecuted = false;
	bIsBound = false;
	ActionDelegate = FScriptDelegate();
}

void UActorIOLink::BindAction(const FActorIOAction& Action)
{
	LinkedAction = Action;

	UActorIOComponent* OwnerIOComponent = GetOwnerIOComponent();
	if (!OwnerIOComponent)
	{
		return;
	}

	const TArray<FActorIOEvent> ValidEvents = UActorIOComponent::GetEventsForObject(OwnerIOComponent->GetOwner());
	const FActorIOEvent* TargetEvent = ValidEvents.FindByKey(LinkedAction.SourceEvent);
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

void UActorIOLink::ClearAction()
{
	UActorIOComponent* OwnerIOComponent = GetOwnerIOComponent();
	if (!OwnerIOComponent)
	{
		return;
	}

	const TArray<FActorIOEvent> ValidEvents = UActorIOComponent::GetEventsForObject(OwnerIOComponent->GetOwner());
	const FActorIOEvent* TargetEvent = ValidEvents.FindByKey(LinkedAction.SourceEvent);
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
	}
}

void UActorIOLink::ExecuteAction()
{
	AActor* TargetActor = LinkedAction.TargetActor.Get();
 	if (!IsValid(TargetActor))
	{
		// The target actor was invalid.
		// Actor was most likely destroyed.
		return;
	}

	TArray<FActorIOFunction> ValidFunctions = UActorIOComponent::GetFunctionsForObject(TargetActor);
	FActorIOFunction* TargetFunction = ValidFunctions.FindByKey(LinkedAction.TargetFunction);
	if (!TargetFunction)
	{
		UE_LOG(LogTemp, Error, TEXT("ActorIOLink: Function '%s' was not found on target actor '%s'"), *LinkedAction.TargetFunction.ToString(), *TargetActor->GetActorNameOrLabel());
		return;
	}

	FString Command = TargetFunction->FunctionToExec;
	Command.Append(TEXT(" "));
	Command.Append(LinkedAction.FunctionArguments);

	UE_LOG(LogTemp, Warning, TEXT("Executing action: %s"), *Command);

	FOutputDeviceNull Ar;
	if (TargetActor->CallFunctionByNameWithArguments(*Command, Ar, this, true))
	{
		bWasExecuted = true;
	}
}

UActorIOComponent* UActorIOLink::GetOwnerIOComponent() const
{
	// ActorIO links are owned by ActorIO components.
	return Cast<UActorIOComponent>(GetOuter());
}

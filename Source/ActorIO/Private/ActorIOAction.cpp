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

	if (!bIsBound)
	{
		ActionDelegate = FScriptDelegate();
		ActionDelegate.BindUFunction(this, TEXT("ExecuteAction"));

		if (TargetEvent->MulticastDelegatePtr)
		{
			TargetEvent->MulticastDelegatePtr->Add(ActionDelegate);
			bIsBound = true;
		}
		else
		{
			FSparseDelegate* SparseDelegate = FSparseDelegateStorage::ResolveSparseDelegate(TargetEvent->DelegateOwner, TargetEvent->SparseDelegateName);
			if (SparseDelegate)
			{
				SparseDelegate->__Internal_AddUnique(TargetEvent->DelegateOwner, TargetEvent->SparseDelegateName, ActionDelegate);
				bIsBound = true;
			}
		}
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
		else
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

void UActorIOLink::ExecuteAction(AActor* OverlappedActor, AActor* OtherActor)
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

// Copyright 2024 Horizon Games. All Rights Reserved.

#include "ActorIOLink.h"
#include "ActorIOComponent.h"
#include "ActorIOInterface.h"
#include "Misc/OutputDeviceNull.h"

UActorIOLink::UActorIOLink()
{
	LinkedAction = FActorIOAction();
	bWasExecuted = false;
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

	const TArray<FActorIOEvent> ValidEvents = OwnerIOComponent->GetEvents();
	const FActorIOEvent* TargetEvent = ValidEvents.FindByKey(LinkedAction.SourceEvent);
	if (!TargetEvent)
	{
		return;
	}

	FMulticastScriptDelegate* TargetDelegate = TargetEvent->EventDelegate;
	if (TargetDelegate)
	{
		ActionDelegate = FScriptDelegate();
		ActionDelegate.BindUFunction(this, TEXT("ExecuteAction"));

		TargetDelegate->Add(ActionDelegate);
	}
	else
	{
		AttemptBindNativeAction();
	}
}

void UActorIOLink::ClearAction()
{
	UActorIOComponent* OwnerIOComponent = GetOwnerIOComponent();
	if (!OwnerIOComponent)
	{
		return;
	}

	const TArray<FActorIOEvent> ValidEvents = OwnerIOComponent->GetEvents();
	const FActorIOEvent* TargetEvent = ValidEvents.FindByKey(LinkedAction.SourceEvent);
	if (!TargetEvent)
	{
		return;
	}

	FMulticastScriptDelegate* TargetDelegate = TargetEvent->EventDelegate;
	if (TargetDelegate)
	{
		if (TargetDelegate->Contains(ActionDelegate))
		{
			TargetDelegate->Remove(ActionDelegate);
			ActionDelegate.Unbind();
		}
	}
}

void UActorIOLink::AttemptBindNativeAction()
{
	ActionDelegate = FScriptDelegate();
	ActionDelegate.BindUFunction(this, TEXT("ExecuteAction"));

	if (LinkedAction.SourceEvent == ToName(EActorIONativeEvents::ActorBeginOverlap))
	{
		AActor* AsActor = GetOwnerIOComponent()->GetOwner();
		AsActor->OnActorBeginOverlap.Add(ActionDelegate);
		return;
	}
	if (LinkedAction.SourceEvent == ToName(EActorIONativeEvents::ActorEndOverlap))
	{
		AActor* AsActor = GetOwnerIOComponent()->GetOwner();
		AsActor->OnActorEndOverlap.Add(ActionDelegate);
		return;
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

	TArray<FActorIOFunction> ValidFunctions = UActorIOComponent::GetNativeFunctionsForObject(TargetActor);
	IActorIOInterface* TargetIO = Cast<IActorIOInterface>(TargetActor);
	if (TargetIO)
	{
		TargetIO->GetActorIOFunctions(ValidFunctions);
	}

	FActorIOFunction* TargetFunction = ValidFunctions.FindByKey(LinkedAction.TargetFunction);
	if (!TargetFunction)
	{
		// Could not find Actor IO function on target actor.
		return;
	}

	FString Command = TargetFunction->FunctionToExec.ToString();
	Command.Append(TEXT(" true"));

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

void UActorIOLink::BeginDestroy()
{
	ClearAction();

	Super::BeginDestroy();
}

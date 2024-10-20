// Copyright 2024 Horizon Games. All Rights Reserved.

#include "ActorIOComponent.h"

void UActorIOComponent::OnRegister()
{
	Super::OnRegister();
}

void UActorIOComponent::AddAction(const FActorIOAction& NewAction)
{
	if (!NewAction.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ActorIOComponent: AddAction failed - The action was invalid!"));
		return;
	}

	if (HasAction(NewAction.ActionId))
	{
		UE_LOG(LogTemp, Error, TEXT("ActorIOComponent: AddAction failed - The action already exists!"));
		return;
	}


}

void UActorIOComponent::RemoveAction(const FGuid& ActionId)
{
	
}

bool UActorIOComponent::GetAction(const FGuid& ActionId, FActorIOAction& OutAction)
{
	return false;
}

bool UActorIOComponent::HasAction(const FGuid& ActionId) const
{
	for (const FActorIOAction& Action : ActionBindings)
	{
		if (Action.ActionId == ActionId)
		{
			return true;
		}
	}

	return false;
}

void UActorIOComponent::BindActions()
{

}

void UActorIOComponent::RemoveActionBindings()
{

}

void UActorIOComponent::OnUnregister()
{
	Super::OnUnregister();
}

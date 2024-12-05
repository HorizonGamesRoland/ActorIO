// Copyright 2024 Horizon Games. All Rights Reserved.

#include "ActorIOMessage.h"
#include "ActorIOAction.h"
#include "Misc/OutputDeviceNull.h"

bool FActorIOMessage::Invoke() const
{
	if (IsValid(TargetObject))
	{
		FOutputDeviceNull Ar;
		return TargetObject->CallFunctionByNameWithArguments(*Command, Ar, OwningAction, true);
	}

	return false;
}

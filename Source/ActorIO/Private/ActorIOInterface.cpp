// Copyright 2024 Horizon Games. All Rights Reserved.

#include "ActorIOInterface.h"

void IActorIOInterface::GetActorIOEvents(TArray<FActorIOEvent>& OutEvents) const
{
	RegisterIOEvents(OutEvents);
	K2_RegisterIOEvents(OutEvents);
}

void IActorIOInterface::GetActorIOFunctions(TArray<FActorIOFunction>& OutFunctions) const
{
	RegisterIOFunctions(OutFunctions);
	K2_RegisterIOFunctions(OutFunctions);
}

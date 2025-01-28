// Copyright 2025 Horizon Games. All Rights Reserved.

#include "ActorIOSettings.h"

UActorIOSettings::UActorIOSettings()
{
	//bLogActionExecution = true;
}

const UActorIOSettings* UActorIOSettings::Get()
{
	return GetDefault<UActorIOSettings>();
}

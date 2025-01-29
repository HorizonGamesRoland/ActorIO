// Copyright 2025 Horizon Games. All Rights Reserved.

#include "ActorIOSettings.h"
#include "ActorIOSubsystemBase.h"

UActorIOSettings::UActorIOSettings()
{
	ActorIOSubsystemClass = UActorIOSubsystemBase::StaticClass();
}

const UActorIOSettings* UActorIOSettings::Get()
{
	return GetDefault<UActorIOSettings>();
}

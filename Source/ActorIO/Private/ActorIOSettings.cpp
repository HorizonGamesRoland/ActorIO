// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO

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

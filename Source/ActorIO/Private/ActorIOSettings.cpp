// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOSettings.h"
#include "ActorIOSubsystemBase.h"

UActorIOSettings::UActorIOSettings()
{
	ActorIOSubsystemClass = UActorIOSubsystemBase::StaticClass();
	LevelActivationMethod = ELevelActivationMethod::Automatic;
}

const UActorIOSettings* UActorIOSettings::Get()
{
	return GetDefault<UActorIOSettings>();
}

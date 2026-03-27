// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOVersions.h"

const FGuid FActorIOActionVersion::GUID(0xD8074578, 0x22004371, 0x8B4120E4, 0x08349C30);

FCustomVersionRegistration GRegisterActorIOActionVersion(FActorIOActionVersion::GUID, FActorIOActionVersion::LatestVersion, TEXT("ActorIOActionVer"));

const FGuid FActorIOMessageVersion::GUID(0x8E6E2EE1, 0x5BCF45F7, 0xBA2DB375, 0x03F86D40);

FCustomVersionRegistration GRegisterActorIOMessageVersion(FActorIOMessageVersion::GUID, FActorIOMessageVersion::LatestVersion, TEXT("ActorIOMessageVer"));
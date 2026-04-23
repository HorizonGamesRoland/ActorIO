// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOVersions.h"
#include "Serialization/CustomVersion.h"

//==================================
//~ Begin FActorIOActionVersion
//==================================

const FGuid FActorIOActionVersion::GUID(0xD8074578, 0x22004371, 0x8B4120E4, 0x08349C30);

FCustomVersionRegistration GRegisterActorIOActionVersion(FActorIOActionVersion::GUID, FActorIOActionVersion::LatestVersion, TEXT("ActorIOActionVer"));

//==================================
//~ Begin FActorIOSubsystemVersion
//==================================

const FGuid FActorIOSubsystemVersion::GUID(0x8E6E2EE1, 0x5BCF45F7, 0xBA2DB375, 0x03F86D40);

FCustomVersionRegistration GRegisterActorIOMessageVersion(FActorIOSubsystemVersion::GUID, FActorIOSubsystemVersion::LatestVersion, TEXT("ActorIOSubsystemVer"));
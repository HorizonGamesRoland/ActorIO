// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "Misc/Guid.h"

/**
 * Custom serialization version for I/O actions and components.
 */
struct FActorIOActionVersion
{
	enum Type
	{
		// Before any version changes were made.
		InitialVersion = 0,

		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};

	/** The GUID for this custom version number. */
	ACTORIO_API const static FGuid GUID;

	/** Not instantiable */
	FActorIOActionVersion() = delete;
};

/**
 * Custom serialization version for the I/O subsystem.
 */
struct FActorIOSubsystemVersion
{
	enum Type
	{
		// Before any version changes were made.
		InitialVersion = 0,

		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};

	/** The GUID for this custom version number. */
	ACTORIO_API const static FGuid GUID;

	/** Not instantiable */
	FActorIOSubsystemVersion() = delete;
};

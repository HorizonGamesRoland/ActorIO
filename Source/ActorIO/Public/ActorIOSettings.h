// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "Engine/DeveloperSettings.h"
#include "Templates/SubclassOf.h"
#include "ActorIOSettings.generated.h"

/** Determines how level activation is handled. */
UENUM()
enum class ELevelActivationMethod : uint8
{
	/** Levels need to be activated and deactivated manually using the I/O subsystem. */
	Manual,
	/** The I/O subsystem activates and deactivates levels automatically when they are added or removed from the world. */
	Automatic
};

/**
 * Settings for the Actor I/O level scripting system.
 */
UCLASS(Config = Game, DefaultConfig)
class ACTORIO_API UActorIOSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:

	/** Default constructor. */
	UActorIOSettings();

	/**
	 * Get the global actor IO plugin settings object.
	 * This returns the class's CDO so every property is read only!
	 */
	static const UActorIOSettings* Get();

public:

	/**
	 * Class to use as the Actor I/O Subsystem.
	 * If none, then the base implementation of the class is used.
	 * Requires a level reload to fully take effect.
	 */
	UPROPERTY(Config, NoClear, EditAnywhere, Category = "Settings", DisplayName = "Actor I/O Subsystem Class")
	TSubclassOf<class UActorIOSubsystemBase> ActorIOSubsystemClass;

	/**
	 * Determines how level activation is handled.
	 * All levels start out in an inactive state, where I/O messages are put into a pending list instead of delivery.
	 * This ensures we do not execute I/O actions before their state can be restored from a save file.
	 * Levels can be activated using the I/O subsystem, which enables I/O message delivery.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	ELevelActivationMethod LevelActivationMethod;

public:

#if WITH_EDITOR
	//~ Begin UDeveloperSettings Interface
	virtual FText GetSectionText() const override { return INVTEXT("Actor I/O"); }
	//~ End UDeveloperSettings Interface
#endif
};

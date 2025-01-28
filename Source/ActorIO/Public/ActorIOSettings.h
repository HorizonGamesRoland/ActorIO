// Copyright 2025 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "Engine/DeveloperSettings.h"
#include "ActorIOSettings.generated.h"

/**
 * Config parameters of the Actor IO plugin.
 */
UCLASS(Config = Game)
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

	//UPROPERTY(Config, EditAnywhere, Category = "Debugging")
	//bool bLogActionExecution;
};

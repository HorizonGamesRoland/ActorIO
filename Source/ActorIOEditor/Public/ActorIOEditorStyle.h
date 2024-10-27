// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

/**
 * ActorIOEditorStyle is a custom style set used by the plugin's editor module.
 */
class ACTORIOEDITOR_API FActorIOEditorStyle
{
private:

	/** Style set used by the plugin. */
	static TUniquePtr<FSlateStyleSet> StyleSet;

public:

	/**
	 * Initializes the style set.
	 * Called during module startup.
	 */
	static void Initialize();

	/**
	 * Resets the style set.
	 * Called during module shutdown.
	 */
	static void Shutdown();

	/** @return The style set used by the plugin. */
	static const ISlateStyle& Get();

	/** @return The name of the style set used by the plugin. */
	static const FName& GetStyleSetName();
};

// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

/**
 * Custom style set used by the Actor I/O editor module.
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

	/** @return Reference to the actual FSlateStyleSet used by the plugin. */
	static FSlateStyleSet* GetMutableStyle();

	/** @return The name of the style set used by the plugin. */
	static const FName& GetStyleSetName();

public:

	/**
	 * Set up default size values for action list columns.
	 * Also used to reset size values to defaults before opening the I/O editor tab.
	 */
	static void SetupActionListColumnSizes();
};

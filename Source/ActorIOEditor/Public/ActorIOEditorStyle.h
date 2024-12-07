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

	/** Height of header rows. */
	static constexpr float HeaderRowHeight = 28.0f;
	/** Height of primary tool buttons. */
	static constexpr float ToolButtonHeight = 38.0f;
	/** Height of action entries. Same as PropertyEditorConstants::PropertyRowHeight. */
	static constexpr float ActionHeight = 26.0f;
	/** Spacing between actions in the action list. */
	static constexpr float ActionSpacing = 3.0f;
	/** Thickness of the action list scrollbar. */
	static constexpr float ActionListScrollbarThickness = 8.0f;

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

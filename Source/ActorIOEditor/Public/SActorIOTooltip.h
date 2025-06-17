// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SToolTip.h"

/**
 * Custom tooltip widget used for I/O events and functions in the editor.
 */
class ACTORIOEDITOR_API SActorIOTooltip : public SToolTip
{
	SLATE_DECLARE_WIDGET(SActorIOTooltip, SToolTip)

public:

	SLATE_BEGIN_ARGS(SActorIOTooltip)
	{}

	/** Id of the I/O event or function. */
	SLATE_ATTRIBUTE(FName, RegistryId)
	/** Description of the I/O event or function. Essentially this is the tooltip text. */
	SLATE_ATTRIBUTE(FText, Description)

	SLATE_END_ARGS()

	/** Widget constructor. */
	void Construct(const FArguments& InArgs);
};

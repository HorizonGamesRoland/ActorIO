// Copyright 2025 Horizon Games. All Rights Reserved.

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

	/** Id that this I/O event or function was registered with. */
	SLATE_ATTRIBUTE(FName, RegistryId)
	/** Description of the I/O event or function. Essentially this is the tooltip text. */
	SLATE_ATTRIBUTE(FText, Description)

	SLATE_END_ARGS()

	/** Widget constructor. */
	void Construct(const FArguments& InArgs);

protected:

	/** Whether the tooltip is empty. If empty, the tooltip will not be displayed. */
	bool bIsTooltipEmpty;

public:

	//~ Begin SToolTip Interface
	virtual bool IsEmpty() const override;
	//~ End SToolTip Interface
};

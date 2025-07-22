// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Notifications/SErrorText.h"
#include "Widgets/SToolTip.h"

/**
 * An error reporting widget displaying an error symbol.
 * The details of the error are displayed in tooltip when hovering the error symbol.
 */
class ACTORIOEDITOR_API SActorIOErrorText : public SErrorText
{
	SLATE_DECLARE_WIDGET(SActorIOErrorText, SErrorText)

public:

	SLATE_BEGIN_ARGS(SActorIOErrorText)
	{}
	SLATE_END_ARGS()

	/** Widget constructor. */
	void Construct(const FArguments& InArgs);

protected:

	/** The error tooltip used for this widget. */
	TSharedPtr<class SActorIOErrorTooltip> ErrorTooltip;

public:

	//~ Begin IErrorReportingWidget Interface
	virtual void SetError(const FText& InErrorText) override;
	virtual TSharedRef<SWidget> AsWidget() override;
	//~ End IErrorReportingWidget Interface
};


/**
 * Custom tooltip widget used for the SActorIOErrorText widget.
 * Displays an error text as tooltip.
 */
class ACTORIOEDITOR_API SActorIOErrorTooltip : public SToolTip
{
	SLATE_DECLARE_WIDGET(SActorIOErrorTooltip, SToolTip)

public:

	SLATE_BEGIN_ARGS(SActorIOErrorTooltip)
	{}
	SLATE_END_ARGS()

	/** Widget constructor. */
	void Construct(const FArguments& InArgs);

	/** Set the tooltip error text. */
	void SetErrorText(const FText& InErrorText);

protected:

	/** The error text inside the tooltip. This is the tooltip content. */
	TSharedPtr<SErrorText> ErrorText;

public:

	//~ Begin SToolTip Interface
	virtual bool IsEmpty() const override;
	//~ End SToolTip Interface
};

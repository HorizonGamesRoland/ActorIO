// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Notifications/SPopUpErrorText.h"
#include "Widgets/SToolTip.h"

/**
 * An error reporting widget displaying error text inside a popup.
 * When the popup is not open, the error is visible inside a tooltip if hovered over the error symbol.
 * Based on SPopupErrorText with the error text being pushed to a custom tooltip widget.
 */
class ACTORIOEDITOR_API SActorIOErrorText : public SPopupErrorText
{
	SLATE_DECLARE_WIDGET(SActorIOErrorText, SPopupErrorText)

public:

	SLATE_BEGIN_ARGS(SActorIOErrorText)
	{}
	SLATE_END_ARGS()

	/**
	 * Widget constructor.
	 * 
	 * For some reason SPopupErrorText::Construct has been made virtual by Epic.
	 * We need to override it to stop the compiler from complaining.
	 */
	void Construct(const FArguments& InArgs);
	virtual void Construct(const SPopupErrorText::FArguments& InArgs) override;

protected:

	/** The error tooltip used for this widget. */
	TSharedPtr<class SActorIOErrorTooltip> PopupErrorTooltip;

public:

	//~ Begin IErrorReportingWidget Interface
	virtual void SetError(const FText& InErrorText) override;
	virtual void SetError(const FString& InErrorText) override;
	virtual TSharedRef<SWidget> AsWidget() override;
	//~ End IErrorReportingWidget Interface
};


/**
 * Custom tooltip widget that looks like an error.
 * Only visible if the owning SActorIOErrorText popup is not open.
 */
class ACTORIOEDITOR_API SActorIOErrorTooltip : public SToolTip
{
	SLATE_DECLARE_WIDGET(SActorIOErrorTooltip, SToolTip)

public:

	SLATE_BEGIN_ARGS(SActorIOErrorTooltip)
	{}

	/** Error text widget that owns this tooltip. */
	SLATE_ARGUMENT(TWeakPtr<SActorIOErrorText>, OwningErrorText)

	SLATE_END_ARGS()

	/** Widget constructor. */
	void Construct(const FArguments& InArgs);

	/**
	 * Set the tooltip error text.
	 * Using an FString because IErrorReportingWidget->SetError always boils down to string version.
	 */
	void SetErrorText(const FString& InErrorText);

protected:

	/** The error text inside the tooltip. This is the tooltip content. */
	TSharedPtr<SErrorText> ErrorText;

	/** Weak reference to error text widget that owns this tooltip. */
	TWeakPtr<SActorIOErrorText> OwningErrorText;

public:

	//~ Begin SToolTip Interface
	virtual bool IsEmpty() const override;
	//~ End SToolTip Interface
};

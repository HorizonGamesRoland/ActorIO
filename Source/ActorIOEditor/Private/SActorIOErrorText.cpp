// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "SActorIOErrorText.h"
#include "Widgets/Notifications/SErrorText.h"

//=======================================================
//~ Begin SActorIOErrorText
//=======================================================

SLATE_IMPLEMENT_WIDGET(SActorIOErrorText)
void SActorIOErrorText::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

void SActorIOErrorText::Construct(const FArguments& InArgs)
{
	// Forward widget construction to SPopupErrorText.
	Construct(SPopupErrorText::FArguments());
}

void SActorIOErrorText::Construct(const SPopupErrorText::FArguments& InArgs)
{
	// This is the construct function of SPopupErrorText.
	// For some reason Epic made this virtual which causes compile issues if we do not override it.
	// We are simply going to construct the SPopupErrorText using the base implementation.

	SPopupErrorText::Construct(InArgs);

	TWeakPtr<SActorIOErrorText> WeakThisPtr = SharedThis(this).ToWeakPtr();
	SetToolTip(SAssignNew(PopupErrorTooltip, SActorIOErrorTooltip).OwningErrorText(WeakThisPtr));
}

void SActorIOErrorText::SetError(const FText& InErrorText)
{
	Super::SetError(InErrorText); // boils down to string version below
}

void SActorIOErrorText::SetError(const FString& InErrorText)
{
	Super::SetError(InErrorText);
	PopupErrorTooltip->SetErrorText(InErrorText);
}

TSharedRef<SWidget> SActorIOErrorText::AsWidget()
{
	return SharedThis(this);
}


//=======================================================
//~ Begin SActorIOErrorTooltip
//=======================================================

SLATE_IMPLEMENT_WIDGET(SActorIOErrorTooltip)
void SActorIOErrorTooltip::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SActorIOErrorTooltip::Construct(const FArguments& InArgs)
{
	OwningErrorText = InArgs._OwningErrorText;

	SToolTip::Construct
	(
		SToolTip::FArguments()
		.BorderImage(FAppStyle::GetBrush("NoBorder"))
		.TextMargin(0.0f)
		.Content()
		[
			SAssignNew(ErrorText, SErrorText)
		]
	);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SActorIOErrorTooltip::SetErrorText(const FString& InErrorText)
{
	ErrorText->SetError(InErrorText);
}

bool SActorIOErrorTooltip::IsEmpty() const
{
	if (OwningErrorText.IsValid())
	{
		TSharedPtr<SActorIOErrorText> TooltipOwner = OwningErrorText.Pin();
		return !TooltipOwner->HasError() || TooltipOwner->IsOpen();
	}

	return true;
}

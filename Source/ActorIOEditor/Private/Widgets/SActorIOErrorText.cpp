// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "Widgets/SActorIOErrorText.h"

//=======================================================
//~ Begin SActorIOErrorText
//=======================================================

SLATE_IMPLEMENT_WIDGET(SActorIOErrorText)
void SActorIOErrorText::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SActorIOErrorText::Construct(const FArguments& InArgs)
{
	SErrorText::Construct(SErrorText::FArguments());
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SActorIOErrorText::SetError(const FText& InErrorText)
{
	const bool bHasError = !InErrorText.IsEmpty();
	Super::SetError(bHasError ? NSLOCTEXT("UnrealEd", "Error", "!") : FText::GetEmpty());

	// This function is auto called during construction so we basically
	// initialize the error tooltip widget here.
	if (!ErrorTooltip.IsValid())
	{
		SetToolTip(SAssignNew(ErrorTooltip, SActorIOErrorTooltip));
	}

	// Push error to tooltip.
	ErrorTooltip->SetErrorText(InErrorText);
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
	SToolTip::Construct(SToolTip::FArguments()
		.BorderImage(FAppStyle::GetBrush("NoBorder"))
		.TextMargin(0.0f)
		.Content()
		[
			SAssignNew(ErrorText, SErrorText)
		]
	);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SActorIOErrorTooltip::SetErrorText(const FText& InErrorText)
{
	ErrorText->SetError(InErrorText);
}

bool SActorIOErrorTooltip::IsEmpty() const
{
	return !ErrorText.IsValid() || !ErrorText->HasError();
}

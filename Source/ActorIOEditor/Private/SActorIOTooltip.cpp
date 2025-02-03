// Copyright 2025 Horizon Games. All Rights Reserved.

#include "SActorIOTooltip.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SSpacer.h"

SLATE_IMPLEMENT_WIDGET(SActorIOTooltip)
void SActorIOTooltip::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SActorIOTooltip::Construct(const FArguments& InArgs)
{
	bIsTooltipEmpty = InArgs._Description.Get().IsEmpty();

	SToolTip::Construct
	(
		SToolTip::FArguments()
		.BorderImage(FCoreStyle::Get().GetBrush("ToolTip.BrightBackground"))
		.TextMargin(11.0f) // border padding
		.Content()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(InArgs._Description.Get())
				.Font(FCoreStyle::Get().GetFontStyle("ToolTip.Font"))
				.ColorAndOpacity(FLinearColor::Black)
				.WrapTextAt_Static(&SToolTip::GetToolTipWrapWidth)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
				.Size(FVector2D(0.0f, 5.0f))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromName(InArgs._RegistryId.Get()))
				.TextStyle(FAppStyle::Get(), "Documentation.SDocumentationTooltipSubdued")
			]
		]
	);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

bool SActorIOTooltip::IsEmpty() const
{
	// This controls whether the tooltip should ever be displayed.
	return bIsTooltipEmpty;
}

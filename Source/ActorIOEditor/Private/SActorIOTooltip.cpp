// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO

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
	const FName& InRegistryId = InArgs._RegistryId.Get();
	const FText& InTooltipText = InArgs._Description.Get();

	// Set tooltip visibility.
	// Used to determine whether the tooltip should ever be displayed.
	bTooltipVisible = InRegistryId.IsNone();

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
				.Text(InTooltipText)
				.Font(FCoreStyle::Get().GetFontStyle("ToolTip.Font"))
				.ColorAndOpacity(FLinearColor::Black)
				.WrapTextAt_Static(&SToolTip::GetToolTipWrapWidth)
				.Visibility(InTooltipText.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible)
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
				.Text(FText::FromName(InRegistryId))
				.TextStyle(FAppStyle::Get(), "Documentation.SDocumentationTooltipSubdued")
			]
		]
	);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

bool SActorIOTooltip::IsEmpty() const
{
	// This controls whether the tooltip should ever be displayed.
	return bTooltipVisible;
}

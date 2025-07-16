// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "Widgets/SActorIOTooltip.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SSpacer.h"
#include "SlateOptMacros.h"

SLATE_IMPLEMENT_WIDGET(SActorIOTooltip)
void SActorIOTooltip::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SActorIOTooltip::Construct(const FArguments& InArgs)
{
	const FName& InRegistryId = InArgs._RegistryId.Get();
	const FText& InDescription = InArgs._Description.Get();

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
				.Text(InDescription)
				.Font(FCoreStyle::Get().GetFontStyle("ToolTip.Font"))
				.ColorAndOpacity(FLinearColor::Black)
				.WrapTextAt_Static(&SToolTip::GetToolTipWrapWidth)
				.Visibility(InDescription.IsEmptyOrWhitespace() ? EVisibility::Collapsed : EVisibility::Visible)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
				.Size(FVector2D(0.0f, 5.0f))
				.Visibility(InDescription.IsEmptyOrWhitespace() ? EVisibility::Collapsed : EVisibility::Visible)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromName(InRegistryId))
				.TextStyle(FAppStyle::Get(), "Documentation.SDocumentationTooltipSubdued")
				.WrapTextAt_Static(&SToolTip::GetToolTipWrapWidth)
			]
		]
	);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

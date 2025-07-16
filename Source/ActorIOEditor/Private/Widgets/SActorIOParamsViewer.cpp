// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "Widgets/SActorIOParamsViewer.h"
#include "ActorIOEditorStyle.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Styling/StyleColors.h"
#include "SlateOptMacros.h"

//=======================================================
//~ Begin SActorIOParamsViewer
//=======================================================

SLATE_IMPLEMENT_WIDGET(SActorIOParamsViewer)
void SActorIOParamsViewer::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SActorIOParamsViewer::Construct(const FArguments& InArgs)
{
	HighlightedParamIdx = INDEX_NONE;

	ChildSlot
	[
		SNew(SBox)
		.MinDesiredWidth(InArgs._MinDesiredWidth)
		[
			SNew(SBorder)
			.Padding(2.0f, 6.0f)
			[
				SAssignNew(ParamsBox, SVerticalBox)
			]
		]
	];

	// Initialize param entries.
	FillParamsBox(InArgs._FunctionPtr);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SActorIOParamsViewer::FillParamsBox(UFunction* InFunctionPtr)
{
	for (TFieldIterator<FProperty> It(InFunctionPtr); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		FProperty* FunctionProp = *It;
		checkSlow(FunctionProp);

		// Do not create widget for return property.
		if (FunctionProp->HasAnyPropertyFlags(CPF_ReturnParm))
		{
			continue;
		}

		// Do not create widget for output params, but only if they are not passed by ref
		// since in that case the value is also an input param.
		if (FunctionProp->HasAnyPropertyFlags(CPF_OutParm) && !FunctionProp->HasAnyPropertyFlags(CPF_ReferenceParm))
		{
			continue;
		}

		TSharedRef<SWidget> ParamWidget = SNew(SActorIOParamsViewerEntry)
			.PropertyPtr(FunctionProp);

		ParamsBox->AddSlot().AttachWidget(ParamWidget);
	}
}

void SActorIOParamsViewer::SetHighlightedParam(int32 InParamIdx)
{
	if (HighlightedParamIdx == InParamIdx)
	{
		// Do nothing if we are trying to highlight the same widget.
		return;
	}

	// Remove highlight from current widget.
	if (IsValidParamIdx(HighlightedParamIdx))
	{
		TSharedRef<SActorIOParamsViewerEntry> HighlightedEntry = GetParamEntryAt(HighlightedParamIdx);
		HighlightedEntry->SetHighlightEnabled(false);
	}

	// Reset the current index here, because there is no guarantee that the new index is valid.
	HighlightedParamIdx = INDEX_NONE;

	// Enable highlight on the other widget.
	if (IsValidParamIdx(InParamIdx))
	{
		TSharedRef<SActorIOParamsViewerEntry> TargetEntry = GetParamEntryAt(InParamIdx);
		TargetEntry->SetHighlightEnabled(true);

		HighlightedParamIdx = InParamIdx;
	}
}

bool SActorIOParamsViewer::IsValidParamIdx(int32 InParamIdx) const
{
	FChildren* Children = ParamsBox->GetChildren();
	return InParamIdx > INDEX_NONE && InParamIdx < Children->Num();
}

TSharedRef<class SActorIOParamsViewerEntry> SActorIOParamsViewer::GetParamEntryAt(int32 InParamIdx) const
{
	FChildren* Children = ParamsBox->GetChildren();
	return StaticCastSharedRef<SActorIOParamsViewerEntry>(Children->GetChildAt(InParamIdx));
}


//=======================================================
//~ Begin SActorIOParamsViewerEntry
//=======================================================

SLATE_IMPLEMENT_WIDGET(SActorIOParamsViewerEntry)
void SActorIOParamsViewerEntry::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SActorIOParamsViewerEntry::Construct(const FArguments& InArgs)
{
	const FProperty* InProperty = InArgs._PropertyPtr;
	checkSlow(InProperty);

	FString ExtendedTypeText;
	FString PropType = InProperty->GetCPPType(&ExtendedTypeText) + ExtendedTypeText;
	FString PropName = InProperty->GetAuthoredName();
	PropName.Append(TEXT(" : "));

	ChildSlot
	[
		SAssignNew(WidgetBorder, SBorder)
		.BorderImage(FActorIOEditorStyle::Get().GetBrush("ParamsViewer.Highlighter"))
		.Padding(6.0f, 3.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(FText::FromString(PropName))
			]
			+ SHorizontalBox::Slot()
			.Padding(1.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(PropType))
				.Font(FAppStyle::GetFontStyle(TEXT("BoldFont")))
				.ColorAndOpacity(FActorIOEditorStyle::Get().GetSlateColor("ParamsViewer.ParamTypeColor"))
			]
		]
	];

	SetHighlightEnabled(false);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SActorIOParamsViewerEntry::SetHighlightEnabled(bool bHighlight)
{
	const FSlateColor& NewColor = bHighlight ? FStyleColors::White : FStyleColors::Transparent;
	WidgetBorder->SetBorderBackgroundColor(NewColor);
}
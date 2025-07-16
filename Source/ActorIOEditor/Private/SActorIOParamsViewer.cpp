// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "SActorIOParamsViewer.h"
#include "ActorIOEditorStyle.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SWrapBox.h"
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
	ChildSlot
	[
		SNew(SBorder)
		//.BorderImage(FActorIOEditorStyle::Get().GetBrush("ParamsViewer.Border"))
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(6.0f)
		[
			SAssignNew(ParamsBox, SWrapBox)
			.PreferredSize(400.0f)
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
	FChildren* Children = ParamsBox->GetChildren();

	// Remove highlight from current widget.
	TSharedPtr<SActorIOParamsViewerEntry> HighlightedEntry = GetHighlightedParam();
	if (HighlightedEntry.IsValid())
	{
		HighlightedEntry->SetHighlightEnabled(false);

		HighlightedParamIdx = INDEX_NONE;
	}

	// Enable highlight on other widget.
	if (InParamIdx > INDEX_NONE && InParamIdx <= Children->Num())
	{
		TSharedRef<SActorIOParamsViewerEntry> ParamsViewerEntry = StaticCastSharedRef<SActorIOParamsViewerEntry>(Children->GetChildAt(InParamIdx));
		ParamsViewerEntry->SetHighlightEnabled(true);

		HighlightedParamIdx = InParamIdx;
	}
}

TSharedPtr<SActorIOParamsViewerEntry> SActorIOParamsViewer::GetHighlightedParam() const
{
	FChildren* Children = ParamsBox->GetChildren();
	if (HighlightedParamIdx > INDEX_NONE && Children->Num() <= HighlightedParamIdx)
	{
		TSharedRef<SActorIOParamsViewerEntry> HighlightedEntry = StaticCastSharedRef<SActorIOParamsViewerEntry>(Children->GetChildAt(HighlightedParamIdx));
		return HighlightedEntry.ToSharedPtr();
	}

	return nullptr;
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
		.BorderImage(FActorIOEditorStyle::Get().GetBrush("ParamsViewer.Highlight"))
		.Padding(3.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(FText::FromString(PropName))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(1.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(PropType))
				.ColorAndOpacity(FStyleColors::PrimaryHover)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT(";")))
			]
		]
	];

	SetHighlightEnabled(false);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SActorIOParamsViewerEntry::SetHighlightEnabled(bool bHighlight)
{
	const FColor HighlightColor = FColor::White.WithAlpha(bHighlight ? 255 : 0);
	WidgetBorder->SetBorderBackgroundColor(FSlateColor(HighlightColor));
}
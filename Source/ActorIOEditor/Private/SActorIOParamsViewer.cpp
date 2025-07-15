// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "SActorIOParamsViewer.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Styling/StyleColors.h"
#include "SlateOptMacros.h"

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
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(11.0f)
		[
			SAssignNew(ParamsBox, SWrapBox)
			.PreferredSize(400.0f)
			.InnerSlotPadding(FVector2D(3.0f))
		]
	];

	FillParamsBox(InArgs._FunctionPtr);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SActorIOParamsViewer::FillParamsBox(const UFunction* InFunctionPtr)
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

		TSharedRef<SWidget> ParamWidget = MakeParamWidgetForProperty(FunctionProp);
		ParamsBox->AddSlot().AttachWidget(ParamWidget);
	}
}

TSharedRef<SWidget> SActorIOParamsViewer::MakeParamWidgetForProperty(const FProperty* InProperty)
{
	FString ExtendedTypeText;
	FString PropType = InProperty->GetCPPType(&ExtendedTypeText) + ExtendedTypeText;
	FString PropName = InProperty->GetAuthoredName();
	PropName.Append(TEXT(" : "));

	return SNew(SHorizontalBox)
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
		];
}

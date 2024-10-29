// Copyright 2024 Horizon Games. All Rights Reserved.

#include "SActorIOAction.h"
#include "ActorIOComponent.h"
#include "ActorIOTypes.h"
#include "ActorIOEditorStyle.h"
#include "PropertyCustomizationHelpers.h"

#define LOCTEXT_NAMESPACE "FActorIOEditor"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SActorIOAction::Construct(const FArguments& InArgs)
{
	IOComponent = InArgs._IOComponent;
	ActionIdx = InArgs._ActionIdx;

	const FActorIOAction& Action = GetAction();

	UpdateSelectableEvents();
	UpdateSelectableFunctions();

	ChildSlot
	[
		SNew(SBox)
		.HeightOverride(26.0f) // PropertyEditorConstants::PropertyRowHeight
		.Padding(0.0f, 0.0f, 0.0f, 2.0f)
		[
			SNew(SSplitter)
			+ SSplitter::Slot()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				.Padding(5.0f, 0.0f)
				[
					SNew(SImage)
					.Image(FActorIOEditorStyle::Get().GetBrush("OutputActionIcon"))
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SComboBox<FName>)
					.OptionsSource(&SelectableEvents)
					.OnGenerateWidget(this, &SActorIOAction::OnGenerateComboBoxWidget)
					.OnSelectionChanged(this, &SActorIOAction::OnEventChanged)
					[
						SAssignNew(EventText, STextBlock)
						.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont")) // PropertyEditorConstants::PropertyFontStyle
						.Text(FText::FromName(Action.SourceEvent))
					]
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				.Padding(5.0f, 0.0f)
				[
					SNew(SImage)
					.Image(FActorIOEditorStyle::Get().GetBrush("ActionArrowIcon"))
				]
				
			]
			+ SSplitter::Slot()
			[
				SNew(SObjectPropertyEntryBox)
				.AllowedClass(AActor::StaticClass())
				.AllowClear(true)
				.DisplayBrowse(true)
				.DisplayUseSelected(false)
				.ObjectPath(this, &SActorIOAction::GetTargetActorPath)
				.OnObjectChanged(this, &SActorIOAction::OnTargetActorChanged)
			]
			+ SSplitter::Slot()
			[
				SNew(SComboBox<FName>)
				.OptionsSource(&SelectableFunctions)
				.OnGenerateWidget(this, &SActorIOAction::OnGenerateComboBoxWidget)
				.OnSelectionChanged(this, &SActorIOAction::OnTargetFunctionChanged)
				[
					SAssignNew(FunctionText, STextBlock)
					.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
					.Text(FText::FromName(Action.TargetFunction))
				]
			]
		]
	];
}

void SActorIOAction::RebuildWidget()
{
	UpdateSelectableEvents();
	UpdateSelectableFunctions();

	const FActorIOAction& Action = GetAction();
	EventText->SetText(FText::FromName(Action.SourceEvent));
	FunctionText->SetText(FText::FromName(Action.TargetFunction));
}

TSharedRef<SWidget> SActorIOAction::OnGenerateComboBoxWidget(FName InName)
{
	return SNew(STextBlock)
		.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont")) // PropertyEditorConstants::PropertyRowHeight
		.Text(FText::FromName(InName));
}

void SActorIOAction::OnEventChanged(FName InName, ESelectInfo::Type InSelectType)
{
	if (InSelectType != ESelectInfo::Direct)
	{
		const FScopedTransaction Transaction(LOCTEXT("ModifyActorIOAction", "Modify ActorIO Action"));
		IOComponent->Modify();

		if (InName == FName(TEXT("<Clear>")))
		{
			InName = NAME_None;
		}

		FActorIOAction& TargetAction = GetAction();
		TargetAction.SourceEvent = InName;

		RebuildWidget();
	}
}

void SActorIOAction::OnTargetFunctionChanged(FName InName, ESelectInfo::Type InSelectType)
{
	if (InSelectType != ESelectInfo::Direct)
	{
		const FScopedTransaction Transaction(LOCTEXT("ModifyActorIOAction", "Modify ActorIO Action"));
		IOComponent->Modify();

		if (InName == FName(TEXT("<Clear>")))
		{
			InName = NAME_None;
		}

		FActorIOAction& Action = GetAction();
		Action.TargetFunction = InName;

		RebuildWidget();
	}
}

FString SActorIOAction::GetTargetActorPath() const
{
	const FActorIOAction& TargetAction = GetAction();
	return TargetAction.TargetActor.GetPathName();
}

void SActorIOAction::OnTargetActorChanged(const FAssetData& InAssetData)
{
	const FScopedTransaction Transaction(LOCTEXT("ModifyActorIOAction", "Modify ActorIO Action"));
	IOComponent->Modify();

	FActorIOAction& TargetAction = GetAction();
	TargetAction.TargetActor = Cast<AActor>(InAssetData.GetAsset());
}

void SActorIOAction::UpdateSelectableEvents()
{
	SelectableEvents.Reset();
	SelectableEvents.Add(TEXT("<Clear>"));

	TArray<FActorIOEvent> ValidEvents = IOComponent->GetEvents();
	for (const FActorIOEvent& IOEvent : ValidEvents)
	{
		SelectableEvents.Emplace(IOEvent.EventName);
	}
}

void SActorIOAction::UpdateSelectableFunctions()
{
	SelectableFunctions.Reset();
	SelectableFunctions.Add(TEXT("<Clear>"));

	TArray<FActorIOFunction> ValidFunctions = IOComponent->GetFunctions();
	for (const FActorIOFunction& IOFunction : ValidFunctions)
	{
		SelectableFunctions.Emplace(IOFunction.FunctionName);
	}
}

FActorIOAction& SActorIOAction::GetAction() const
{
	check(IOComponent && IOComponent->GetActions().IsValidIndex(ActionIdx));
	return IOComponent->GetActions()[ActionIdx];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE

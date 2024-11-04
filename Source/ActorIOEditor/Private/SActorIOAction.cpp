// Copyright 2024 Horizon Games. All Rights Reserved.

#include "SActorIOAction.h"
#include "ActorIOAction.h"
#include "ActorIOComponent.h"
#include "ActorIOInterface.h"
#include "ActorIOEditor.h"
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
					.OptionsSource(&SelectableEventIds)
					.OnGenerateWidget(this, &SActorIOAction::OnGenerateEventComboBoxWidget)
					.OnSelectionChanged(this, &SActorIOAction::OnEventChanged)
					[
						SAssignNew(EventText, STextBlock)
						.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont")) // PropertyEditorConstants::PropertyFontStyle
						.Text(GetEventDisplayName(Action.SourceEvent))
						.ColorAndOpacity(DoesEventExist(Action.SourceEvent) ? FColor::White : FColor::Red)
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
				.OptionsSource(&SelectableFunctionIds)
				.OnGenerateWidget(this, &SActorIOAction::OnGenerateFunctionComboBoxWidget)
				.OnSelectionChanged(this, &SActorIOAction::OnTargetFunctionChanged)
				[
					SAssignNew(FunctionText, STextBlock)
					.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
					.Text(GetFunctionDisplayName(Action.TargetFunction))
					.ColorAndOpacity(DoesFunctionExist(Action.TargetFunction) ? FColor::White : FColor::Red)
				]
			]
			+ SSplitter::Slot()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(0.0f, 0.0f, 3.0f, 0.0f)
				[
					SNew(SEditableTextBox)
					.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
					.Text(FText::FromString(Action.FunctionArguments))
					.OnTextCommitted(this, &SActorIOAction::OnFunctionArgumentsChanged)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.ContentPadding(0.0f)
					.Text(LOCTEXT("RemoveAction", "X"))
					.OnClicked(this, &SActorIOAction::OnClick_RemoveAction)
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
	EventText->SetText(GetEventDisplayName(Action.SourceEvent));
	EventText->SetColorAndOpacity(DoesEventExist(Action.SourceEvent) ? FColor::White : FColor::Red);

	FunctionText->SetText(GetFunctionDisplayName(Action.TargetFunction));
	FunctionText->SetColorAndOpacity(DoesFunctionExist(Action.TargetFunction) ? FColor::White : FColor::Red);
}

TSharedRef<SWidget> SActorIOAction::OnGenerateEventComboBoxWidget(FName InName)
{
	return SNew(STextBlock)
		.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont")) // PropertyEditorConstants::PropertyFontStyle
		.Text(GetEventDisplayName(InName))
		.ToolTipText(GetEventTooltipText(InName));
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

TSharedRef<SWidget> SActorIOAction::OnGenerateFunctionComboBoxWidget(FName InName)
{
	return SNew(STextBlock)
		.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont")) // PropertyEditorConstants::PropertyFontStyle
		.Text(GetFunctionDisplayName(InName))
		.ToolTipText(GetFunctionTooltipText(InName));
}

void SActorIOAction::OnTargetActorChanged(const FAssetData& InAssetData)
{
	const FScopedTransaction Transaction(LOCTEXT("ModifyActorIOAction", "Modify ActorIO Action"));
	IOComponent->Modify();

	FActorIOAction& TargetAction = GetAction();
	TargetAction.TargetActor = Cast<AActor>(InAssetData.GetAsset());

	TargetAction.TargetFunction = NAME_None;
	TargetAction.FunctionArguments = FString();

	UpdateSelectableFunctions();
}

FString SActorIOAction::GetTargetActorPath() const
{
	const FActorIOAction& TargetAction = GetAction();
	return TargetAction.TargetActor.GetPathName();
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
		Action.FunctionArguments = FString();

		RebuildWidget();
	}
}

void SActorIOAction::OnFunctionArgumentsChanged(const FText& InText, ETextCommit::Type InCommitType)
{
	const FScopedTransaction Transaction(LOCTEXT("ModifyActorIOAction", "Modify ActorIO Action"));
	IOComponent->Modify();

	FActorIOAction& TargetAction = GetAction();
	TargetAction.FunctionArguments = InText.ToString();
}

FReply SActorIOAction::OnClick_RemoveAction()
{
	const FScopedTransaction Transaction(LOCTEXT("RemoveActorIOAction", "Remove ActorIO Action"));
	IOComponent->Modify();

	IOComponent->GetActions().RemoveAt(ActionIdx);

	FActorIOEditor& ActorIOEditorModule = FModuleManager::GetModuleChecked<FActorIOEditor>("ActorIOEditor");
	ActorIOEditorModule.UpdateEditorWindow();

	return FReply::Handled();
}

void SActorIOAction::UpdateSelectableEvents()
{
	SelectableEventIds.Reset();
	SelectableEventIds.Add(TEXT("<Clear>"));

	ValidEvents = UActorIOComponent::GetEventsForObject(IOComponent->GetOwner());
	for (const FActorIOEvent& IOEvent : ValidEvents)
	{
		SelectableEventIds.Emplace(IOEvent.EventId);
	}
}

void SActorIOAction::UpdateSelectableFunctions()
{
	SelectableFunctionIds.Reset();
	SelectableFunctionIds.Add(TEXT("<Clear>"));

	ValidFunctions = UActorIOComponent::GetFunctionsForObject(GetAction().TargetActor);
	for (const FActorIOFunction& IOFunction : ValidFunctions)
	{
		SelectableFunctionIds.Emplace(IOFunction.FunctionId);
	}
}

FActorIOAction& SActorIOAction::GetAction() const
{
	check(IOComponent && IOComponent->GetActions().IsValidIndex(ActionIdx));
	return IOComponent->GetActions()[ActionIdx];
}

FText SActorIOAction::GetEventDisplayName(FName InEventId) const
{
	const FActorIOEvent* TargetEvent = ValidEvents.FindByKey(InEventId);
	if (TargetEvent && !TargetEvent->DisplayName.IsEmpty())
	{
		return TargetEvent->DisplayName;
	}

	return FText::FromName(InEventId);
}

FText SActorIOAction::GetEventTooltipText(FName InEventId) const
{
	const FActorIOEvent* TargetEvent = ValidEvents.FindByKey(InEventId);
	if (TargetEvent)
	{
		return TargetEvent->TooltipText;
	}

	return FText::GetEmpty();
}

FText SActorIOAction::GetFunctionDisplayName(FName InFunctionId) const
{
	const FActorIOFunction* TargetFunction = ValidFunctions.FindByKey(InFunctionId);
	if (TargetFunction && !TargetFunction->DisplayName.IsEmpty())
	{
		return TargetFunction->DisplayName;
	}

	return FText::FromName(InFunctionId);
}

FText SActorIOAction::GetFunctionTooltipText(FName InFunctionId) const
{
	const FActorIOFunction* TargetFunction = ValidFunctions.FindByKey(InFunctionId);
	if (TargetFunction)
	{
		return TargetFunction->TooltipText;
	}

	return FText::GetEmpty();
}

bool SActorIOAction::DoesEventExist(FName InEventId) const
{
	if (InEventId == NAME_None)
	{
		// Also accept 'None' as valid because we only want to highlight outdated events/functions.
		return true;
	}

	const FActorIOEvent* TargetEvent = ValidEvents.FindByKey(InEventId);
	return TargetEvent != nullptr;
}

bool SActorIOAction::DoesFunctionExist(FName InFunctionId) const
{
	if (InFunctionId == NAME_None)
	{
		// Also accept 'None' as valid because we only want to highlight outdated events/functions.
		return true;
	}

	const FActorIOFunction* TargetFunction = ValidFunctions.FindByKey(InFunctionId);
	return TargetFunction != nullptr;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE

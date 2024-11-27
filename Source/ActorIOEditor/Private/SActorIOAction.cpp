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

SLATE_IMPLEMENT_WIDGET(SActorIOAction)
void SActorIOAction::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

void SActorIOAction::Construct(const FArguments& InArgs)
{
	Action = InArgs._Action;

	UpdateSelectableEvents();
	UpdateSelectableFunctions();

	ChildSlot
	[
		SNew(SBox)
		.HeightOverride(26.0f) // PropertyEditorConstants::PropertyRowHeight
		.Padding(0.0f, 0.0f, 0.0f, 2.0f)
		[
			SAssignNew(PropertySplitter, SSplitter)
			.PhysicalSplitterHandleSize(0.0f)
		]
	];

	InitializeAction();

	// Apply property sizes from the IO editor.
	for (int32 SlotIdx = 0; SlotIdx != InArgs._PropertySizes.Num(); ++SlotIdx)
	{
		SetPropertySize(SlotIdx, InArgs._PropertySizes[SlotIdx]);
	}
}

void SActorIOAction::Refresh()
{
	UpdateSelectableEvents();
	UpdateSelectableFunctions();
}

void SActorIOAction::SetPropertySize(int32 SlotIdx, float InSize)
{
	const int32 NumSlots = PropertySplitter->GetChildren()->Num();
	if (ensureMsgf(NumSlots - 1 >= SlotIdx, TEXT("Action does not have same amount of slots as the header row in IO tab.")))
	{
		PropertySplitter->SlotAt(SlotIdx).SetSizeValue(InSize);
	}
}

void SActorIOAction::UpdateSelectableEvents()
{
	SelectableEventIds.Reset();
	SelectableEventIds.Add(TEXT("<Clear>"));

	ValidEvents = UActorIOComponent::GetEventsForObject(Action->GetOwnerActor());
	for (const FActorIOEvent& IOEvent : ValidEvents)
	{
		SelectableEventIds.Emplace(IOEvent.EventId);
	}
}

void SActorIOAction::UpdateSelectableFunctions()
{
	SelectableFunctionIds.Reset();
	SelectableFunctionIds.Add(TEXT("<Clear>"));

	ValidFunctions = UActorIOComponent::GetFunctionsForObject(Action->TargetActor);
	for (const FActorIOFunction& IOFunction : ValidFunctions)
	{
		SelectableFunctionIds.Emplace(IOFunction.FunctionId);
	}
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

FColor SActorIOAction::GetEventTextColor(FName InEventId) const
{
	if (InEventId == NAME_None)
	{
		// Also accept 'None' as valid because we only want to highlight outdated events/functions.
		return FColor::White;
	}

	const FActorIOEvent* TargetEvent = ValidEvents.FindByKey(InEventId);
	return TargetEvent ? FColor::White : FColor::Red;
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

FColor SActorIOAction::GetFunctionTextColor(FName InFunctionId) const
{
	if (InFunctionId == NAME_None)
	{
		// Also accept 'None' as valid because we only want to highlight outdated events/functions.
		return FColor::White;
	}

	const FActorIOFunction* TargetFunction = ValidFunctions.FindByKey(InFunctionId);
	return TargetFunction ? FColor::White : FColor::Red;
}



SLATE_IMPLEMENT_WIDGET(SActorOutputAction)
void SActorOutputAction::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

void SActorOutputAction::Construct(const FArguments& InArgs)
{
	Super::Construct(SActorIOAction::FArguments()
		.Action(InArgs._Action)
		.PropertySizes(InArgs._PropertySizes));
}

void SActorOutputAction::InitializeAction()
{
	PropertySplitter->AddSlot()
	.Resizable(false)
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
			.OnGenerateWidget(this, &SActorOutputAction::OnGenerateEventComboBoxWidget)
			.OnSelectionChanged(this, &SActorOutputAction::OnEventChanged)
			[
				SAssignNew(EventText, STextBlock)
				.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont")) // PropertyEditorConstants::PropertyFontStyle
				.Text(GetEventDisplayName(Action->EventId))
				.ColorAndOpacity(GetEventTextColor(Action->EventId))
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
	];

	PropertySplitter->AddSlot()
	.Resizable(false)
	[
		SNew(SObjectPropertyEntryBox)
		.AllowedClass(AActor::StaticClass())
		.AllowClear(true)
		.DisplayBrowse(true)
		.DisplayUseSelected(false)
		.ObjectPath(this, &SActorOutputAction::GetTargetActorPath)
		.OnObjectChanged(this, &SActorOutputAction::OnTargetActorChanged)
	];

	PropertySplitter->AddSlot()
	.Resizable(false)
	[
		SNew(SComboBox<FName>)
		.OptionsSource(&SelectableFunctionIds)
		.OnGenerateWidget(this, &SActorOutputAction::OnGenerateFunctionComboBoxWidget)
		.OnSelectionChanged(this, &SActorOutputAction::OnTargetFunctionChanged)
		[
			SAssignNew(FunctionText, STextBlock)
			.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
			.Text(GetFunctionDisplayName(Action->FunctionId))
			.ColorAndOpacity(GetFunctionTextColor(Action->FunctionId))
		]
	];

	PropertySplitter->AddSlot()
	.Resizable(false)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.Padding(3.0f, 0.0f, 3.0f, 0.0f)
		[
			SNew(SEditableTextBox)
			.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
			.Text(FText::FromString(Action->FunctionArguments))
			.OnTextCommitted(this, &SActorOutputAction::OnFunctionArgumentsChanged)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.ContentPadding(0.0f)
			.Text(LOCTEXT("RemoveAction", "X"))
			.OnClicked(this, &SActorOutputAction::OnClick_RemoveAction)
		]
	];
}

void SActorOutputAction::Refresh()
{
	Super::Refresh();

	EventText->SetText(GetEventDisplayName(Action->EventId));
	EventText->SetColorAndOpacity(GetEventTextColor(Action->EventId));

	FunctionText->SetText(GetFunctionDisplayName(Action->FunctionId));
	FunctionText->SetColorAndOpacity(GetFunctionTextColor(Action->FunctionId));
}

TSharedRef<SWidget> SActorOutputAction::OnGenerateEventComboBoxWidget(FName InName)
{
	return SNew(STextBlock)
		.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont")) // PropertyEditorConstants::PropertyFontStyle
		.Text(GetEventDisplayName(InName))
		.ToolTipText(GetEventTooltipText(InName));
}

TSharedRef<SWidget> SActorOutputAction::OnGenerateFunctionComboBoxWidget(FName InName)
{
	return SNew(STextBlock)
		.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont")) // PropertyEditorConstants::PropertyFontStyle
		.Text(GetFunctionDisplayName(InName))
		.ToolTipText(GetFunctionTooltipText(InName));
}

void SActorOutputAction::OnEventChanged(FName InName, ESelectInfo::Type InSelectType)
{
	if (InSelectType != ESelectInfo::Direct)
	{
		const FScopedTransaction Transaction(LOCTEXT("ModifyActorIOAction", "Modify ActorIO Action"));
		Action->Modify();

		UActorIOComponent* ActionOwner = Action->GetOwnerIOComponent();
		ActionOwner->Modify();

		if (InName == FName(TEXT("<Clear>")))
		{
			InName = NAME_None;
		}

		Action->EventId = InName;

		Refresh();
	}
}

void SActorOutputAction::OnTargetActorChanged(const FAssetData& InAssetData)
{
	const FScopedTransaction Transaction(LOCTEXT("ModifyActorIOAction", "Modify ActorIO Action"));
	Action->Modify();

	UActorIOComponent* ActionOwner = Action->GetOwnerIOComponent();
	ActionOwner->Modify();

	Action->TargetActor = Cast<AActor>(InAssetData.GetAsset());
	Action->FunctionId = NAME_None;
	Action->FunctionArguments = FString();

	UpdateSelectableFunctions();
}

FString SActorOutputAction::GetTargetActorPath() const
{
	return Action->TargetActor.GetPathName();
}

void SActorOutputAction::OnTargetFunctionChanged(FName InName, ESelectInfo::Type InSelectType)
{
	if (InSelectType != ESelectInfo::Direct)
	{
		const FScopedTransaction Transaction(LOCTEXT("ModifyActorIOAction", "Modify ActorIO Action"));
		Action->Modify();

		UActorIOComponent* ActionOwner = Action->GetOwnerIOComponent();
		ActionOwner->Modify();

		if (InName == FName(TEXT("<Clear>")))
		{
			InName = NAME_None;
		}

		Action->FunctionId = InName;
		Action->FunctionArguments = FString();

		Refresh();
	}
}

void SActorOutputAction::OnFunctionArgumentsChanged(const FText& InText, ETextCommit::Type InCommitType)
{
	const FScopedTransaction Transaction(LOCTEXT("ModifyActorIOAction", "Modify ActorIO Action"));
	Action->Modify();

	UActorIOComponent* ActionOwner = Action->GetOwnerIOComponent();
	ActionOwner->Modify();

	Action->FunctionArguments = InText.ToString();
}

FReply SActorOutputAction::OnClick_RemoveAction()
{
	const FScopedTransaction Transaction(LOCTEXT("RemoveActorIOAction", "Remove ActorIO Action"));

	UActorIOComponent* ActionOwner = Action->GetOwnerIOComponent();
	ActionOwner->Modify();

	ActionOwner->GetActions().Remove(Action);

	FActorIOEditor& ActorIOEditorModule = FModuleManager::GetModuleChecked<FActorIOEditor>("ActorIOEditor");
	ActorIOEditorModule.UpdateEditorWindow();

	return FReply::Handled();
}


SLATE_IMPLEMENT_WIDGET(SActorInputAction)
void SActorInputAction::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

void SActorInputAction::Construct(const FArguments& InArgs)
{
	Super::Construct(SActorIOAction::FArguments()
		.Action(InArgs._Action)
		.PropertySizes(InArgs._PropertySizes));
}

void SActorInputAction::InitializeAction()
{
	PropertySplitter->AddSlot()
	.Resizable(false)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		.Padding(5.0f, 0.0f)
		[
			SNew(SImage)
			.Image(FActorIOEditorStyle::Get().GetBrush("InputActionIcon"))
		]
		+ SHorizontalBox::Slot()
		[
			SAssignNew(EventText, STextBlock)
			.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont")) // PropertyEditorConstants::PropertyFontStyle
			.Text(FText::FromString(TEXT("Input action")))
		]
	];
}

void SActorInputAction::Refresh()
{
	Super::Refresh();

	EventText->SetText(GetEventDisplayName(Action->EventId));
	EventText->SetColorAndOpacity(GetEventTextColor(Action->EventId));

	FunctionText->SetText(GetFunctionDisplayName(Action->FunctionId));
	FunctionText->SetColorAndOpacity(GetFunctionTextColor(Action->FunctionId));
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
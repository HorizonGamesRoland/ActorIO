// Copyright 2024 Horizon Games. All Rights Reserved.

#include "SActorIOActionList.h"
#include "SActorIOAction.h"
#include "ActorIOSystem.h"
#include "ActorIOComponent.h"
#include "ActorIOAction.h"
#include "ActorIOEditor.h"
#include "ActorIOEditorSubsystem.h"
#include "ActorIOEditorStyle.h"
#include "PropertyCustomizationHelpers.h"
#include "Widgets/Input/SSpinBox.h"

#define LOCTEXT_NAMESPACE "ActorIOEditor"

//=======================================================
//~ Begin SActorIOActionListView
//=======================================================

SLATE_IMPLEMENT_WIDGET(SActorIOActionListView)
void SActorIOActionListView::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SActorIOActionListView::Construct(const FArguments& InArgs)
{
    SListView::Construct
	(
        SListView::FArguments()
		.ListViewStyle(&FActorIOEditorStyle::Get().GetWidgetStyle<FTableViewStyle>("ActionListView"))
        .ListItemsSource(&ActionListItems)
		.SelectionMode(ESelectionMode::None)
        .OnGenerateRow(this, &SActorIOActionListView::OnGenerateWidgetForActionListView)
        .HeaderRow
        (
            SNew(SHeaderRow)
			+ SHeaderRow::Column(ColumnId::ActionType)
			.DefaultLabel(FText::GetEmpty())
			.FixedWidth(30.0f)
			[
				SNew(SBox)
				.HeightOverride(FActorIOEditorStyle::HeaderRowHeight)
			]

            +SHeaderRow::Column(ColumnId::Event)
            .DefaultLabel(LOCTEXT("ActionListColEvent", "Event"))
			.DefaultTooltip(FText::GetEmpty())
            .FillWidth(1.0f)
			.HeaderContentPadding(FMargin(5.0f, 0.0f, 0.0f, 0.0f))

            + SHeaderRow::Column(ColumnId::Target)
            .DefaultLabel(LOCTEXT("ActionListColTarget", "Target"))
			.DefaultTooltip(FText::GetEmpty())
            .FillWidth(1.0f)
			.HeaderContentPadding(FMargin(5.0f, 0.0f, 0.0f, 0.0f))

            + SHeaderRow::Column(ColumnId::Action)
            .DefaultLabel(LOCTEXT("ActionListColAction", "Action"))
			.DefaultTooltip(FText::GetEmpty())
            .FillWidth(1.0f)
			.HeaderContentPadding(FMargin(5.0f, 0.0f, 0.0f, 0.0f))

            + SHeaderRow::Column(ColumnId::Parameter)
            .DefaultLabel(LOCTEXT("ActionListColParameter", "Parameter"))
			.DefaultTooltip(FText::GetEmpty())
            .FillWidth(1.0f)
			.HeaderContentPadding(FMargin(5.0f, 0.0f, 0.0f, 0.0f))

            + SHeaderRow::Column(ColumnId::Delay)
            .DefaultLabel(LOCTEXT("ActionListColDelay", "Delay"))
			.DefaultTooltip(FText::GetEmpty())
            .FillWidth(0.35f)
			.HeaderContentPadding(FMargin(5.0f, 0.0f, 0.0f, 0.0f))

            + SHeaderRow::Column(ColumnId::OnlyOnce)
            .DefaultLabel(LOCTEXT("ActionListColOnce", "Once?"))
			.DefaultTooltip(FText::GetEmpty())
            .FillWidth(0.5f)
			.HeaderContentPadding(FMargin(5.0f, 0.0f, 0.0f, 0.0f))
        )
    );

    Refresh();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SActorIOActionListView::Refresh()
{
	ActionListItems.Reset();

	UActorIOEditorSubsystem* ActorIOEditorSubsystem = GEditor->GetEditorSubsystem<UActorIOEditorSubsystem>();
	AActor* SelectedActor = ActorIOEditorSubsystem ? ActorIOEditorSubsystem->GetSelectedActor() : nullptr;
	UActorIOComponent* ActorIOComponent = SelectedActor ? SelectedActor->GetComponentByClass<UActorIOComponent>() : nullptr;

	if (ActorIOComponent)
	{
		ActionListItems = ActorIOComponent->GetActions();
	}

	RequestListRefresh();
}

TSharedRef<ITableRow> SActorIOActionListView::OnGenerateWidgetForActionListView(TWeakObjectPtr<UActorIOAction> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(SActorIOActionListViewRow, OwnerTable, Item);
}


//=======================================================
//~ Begin SActorIOActionListViewRow
//=======================================================

SLATE_IMPLEMENT_WIDGET(SActorIOActionListViewRow)
void SActorIOActionListViewRow::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

void SActorIOActionListViewRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, TWeakObjectPtr<UActorIOAction> InActionPtr)
{
	ActionPtr = InActionPtr;

	UpdateSelectableEvents();
	UpdateSelectableFunctions();

	FSuperRowType::Construct(FTableRowArgs(), InOwnerTableView);
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
TSharedRef<SWidget> SActorIOActionListViewRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	TSharedRef<SBox> OutWidget = SNew(SBox);

	if (!ActionPtr.IsValid())
	{
		OutWidget->SetPadding(FMargin(0.0f, FActorIOEditorStyle::ActionSpacing, 0.0f, 0.0f));
		OutWidget->SetContent
		(
			SNew(STextBlock)
			.Text(INVTEXT("Invalid Action!"))
			.ColorAndOpacity(FStyleColors::Error)
		);
	}
	else if (ColumnName == ColumnId::ActionType)
	{
		OutWidget->SetHAlign(HAlign_Center);
		OutWidget->SetVAlign(VAlign_Center);
		OutWidget->SetPadding(FMargin(0.0f, FActorIOEditorStyle::ActionSpacing, 0.0f, 0.0f));
		OutWidget->SetContent
		(
			SNew(SImage)
			.Image(FActorIOEditorStyle::Get().GetBrush("Action.OutputIcon"))
		);
	}
	else if (ColumnName == ColumnId::Event)
	{
		OutWidget->SetPadding(FMargin(0.0f, FActorIOEditorStyle::ActionSpacing, 0.0f, 0.0f));
		OutWidget->SetContent
		(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(SComboBox<FName>)
				.OptionsSource(&SelectableEventIds)
				.OnGenerateWidget(this, &SActorIOActionListViewRow::OnGenerateEventComboBoxWidget)
				.OnComboBoxOpening(this, &SActorIOActionListViewRow::OnEventComboBoxOpening)
				.OnSelectionChanged(this, &SActorIOActionListViewRow::OnEventComboBoxSelectionChanged)
				[
					SAssignNew(EventText, STextBlock)
					.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont")) // PropertyEditorConstants::PropertyFontStyle
					.Text(GetEventDisplayName(ActionPtr->EventId))
					.ColorAndOpacity(GetEventTextColor(ActionPtr->EventId))
				]
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			.Padding(5.0f, 0.0f)
			[
				SNew(SImage)
				.Image(FActorIOEditorStyle::Get().GetBrush("Action.TargetIcon"))
			]
		);
	}
	else if (ColumnName == ColumnId::Target)
	{
		OutWidget->SetPadding(FMargin(0.0f, FActorIOEditorStyle::ActionSpacing, 0.0f, 0.0f));
		OutWidget->SetContent
		(
			SNew(SObjectPropertyEntryBox)
			.AllowedClass(AActor::StaticClass())
			.AllowClear(true)
			.EnableContentPicker(true)
			.DisplayBrowse(false)
			.DisplayUseSelected(false)
			.ObjectPath(this, &SActorIOActionListViewRow::OnGetTargetActorPath)
			.OnObjectChanged(this, &SActorIOActionListViewRow::OnTargetActorChanged)
		);
	}
	else if (ColumnName == ColumnId::Action)
	{
		OutWidget->SetPadding(FMargin(0.0f, FActorIOEditorStyle::ActionSpacing, 1.0f, 0.0f));
		OutWidget->SetContent
		(
			SNew(SComboBox<FName>)
			.OptionsSource(&SelectableFunctionIds)
			.OnGenerateWidget(this, &SActorIOActionListViewRow::OnGenerateFunctionComboBoxWidget)
			.OnComboBoxOpening(this, &SActorIOActionListViewRow::OnFunctionComboBoxOpening)
			.OnSelectionChanged(this, &SActorIOActionListViewRow::OnFunctionComboBoxSelectionChanged)
			[
				SAssignNew(FunctionText, STextBlock)
				.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
				.Text(GetFunctionDisplayName(ActionPtr->FunctionId))
				.ColorAndOpacity(GetFunctionTextColor(ActionPtr->FunctionId))
			]
		);
	}
	else if (ColumnName == ColumnId::Parameter)
	{
		OutWidget->SetPadding(FMargin(1.0f, FActorIOEditorStyle::ActionSpacing, 1.0f, 0.0f));
		OutWidget->SetContent
		(
			SNew(SEditableTextBox)
			.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
			.Text(FText::FromString(ActionPtr->FunctionArguments))
			.OnTextCommitted(this, &SActorIOActionListViewRow::OnFunctionParametersChanged)
		);
	}
	else if (ColumnName == ColumnId::Delay)
	{
		OutWidget->SetPadding(FMargin(1.0f, FActorIOEditorStyle::ActionSpacing, 1.0f, 0.0f));
		OutWidget->SetContent
		(
			SNew(SSpinBox<float>)
			.MinValue(0.0f)
			.MinFractionalDigits(2)
			.MaxFractionalDigits(2)
			.EnableSlider(false)
			.EnableWheel(false)
			.AlwaysUsesDeltaSnap(true)
			.Delta(0.01f)
			.Value(this, &SActorIOActionListViewRow::OnGetActionDelay)
			.OnValueCommitted(this, &SActorIOActionListViewRow::OnActionDelayChanged)
		);
	}
	else if (ColumnName == ColumnId::OnlyOnce)
	{
		OutWidget->SetPadding(FMargin(3.0f, FActorIOEditorStyle::ActionSpacing, 0.0f, 0.0f));
		OutWidget->SetContent
		(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SCheckBox)
				.IsChecked(this, &SActorIOActionListViewRow::IsExecuteOnlyOnceChecked)
				.OnCheckStateChanged(this, &SActorIOActionListViewRow::OnExecuteOnlyOnceChecked)
			]
			+ SHorizontalBox::Slot()
			[
				SNew(SSpacer)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 2.0f, 0.0f)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.ContentPadding(0.0f)
				.OnClicked(this, &SActorIOActionListViewRow::OnClick_RemoveAction)
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("Icons.Delete"))
					.Visibility(EVisibility::HitTestInvisible)
				]
			]
		);
	}

	return OutWidget;
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SActorIOActionListViewRow::UpdateSelectableEvents()
{
	SelectableEventIds.Reset();
	SelectableEventIds.Add(TEXT("<Clear>"));

	ValidEvents = UActorIOSystem::GetEventsForObject(ActionPtr->GetOwnerActor());
	for (const FActorIOEvent& IOEvent : ValidEvents)
	{
		SelectableEventIds.Emplace(IOEvent.EventId);
	}
}

void SActorIOActionListViewRow::UpdateSelectableFunctions()
{
	SelectableFunctionIds.Reset();
	SelectableFunctionIds.Add(TEXT("<Clear>"));

	ValidFunctions = UActorIOSystem::GetFunctionsForObject(ActionPtr->TargetActor);
	for (const FActorIOFunction& IOFunction : ValidFunctions)
	{
		SelectableFunctionIds.Emplace(IOFunction.FunctionId);
	}
}

FText SActorIOActionListViewRow::GetEventDisplayName(FName InEventId) const
{
	const FActorIOEvent* TargetEvent = ValidEvents.FindByKey(InEventId);
	if (TargetEvent && !TargetEvent->DisplayName.IsEmpty())
	{
		return TargetEvent->DisplayName;
	}

	return FText::FromName(InEventId);
}

FText SActorIOActionListViewRow::GetEventTooltipText(FName InEventId) const
{
	const FActorIOEvent* TargetEvent = ValidEvents.FindByKey(InEventId);
	if (TargetEvent)
	{
		return TargetEvent->TooltipText;
	}

	return FText::GetEmpty();
}

FSlateColor SActorIOActionListViewRow::GetEventTextColor(FName InEventId) const
{
	if (InEventId == NAME_None)
	{
		// Also accept 'None' as valid because we only want to highlight outdated events/functions.
		return FSlateColor::UseForeground();
	}

	const FActorIOEvent* TargetEvent = ValidEvents.FindByKey(InEventId);
	return TargetEvent ? FSlateColor::UseForeground() : FStyleColors::Error;
}

FText SActorIOActionListViewRow::GetFunctionDisplayName(FName InFunctionId) const
{
	const FActorIOFunction* TargetFunction = ValidFunctions.FindByKey(InFunctionId);
	if (TargetFunction && !TargetFunction->DisplayName.IsEmpty())
	{
		return TargetFunction->DisplayName;
	}

	return FText::FromName(InFunctionId);
}

FText SActorIOActionListViewRow::GetFunctionTooltipText(FName InFunctionId) const
{
	const FActorIOFunction* TargetFunction = ValidFunctions.FindByKey(InFunctionId);
	if (TargetFunction)
	{
		return TargetFunction->TooltipText;
	}

	return FText::GetEmpty();
}

FSlateColor SActorIOActionListViewRow::GetFunctionTextColor(FName InFunctionId) const
{
	if (InFunctionId == NAME_None)
	{
		// Also accept 'None' as valid because we only want to highlight outdated events/functions.
		return FSlateColor::UseForeground();
	}

	const FActorIOFunction* TargetFunction = ValidFunctions.FindByKey(InFunctionId);
	return TargetFunction ? FSlateColor::UseForeground() : FStyleColors::Error;
}

TSharedRef<SWidget> SActorIOActionListViewRow::OnGenerateEventComboBoxWidget(FName InName) const
{
	return SNew(STextBlock)
		.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont")) // PropertyEditorConstants::PropertyFontStyle
		.Text(GetEventDisplayName(InName))
		.ToolTipText(GetEventTooltipText(InName));
}

void SActorIOActionListViewRow::OnEventComboBoxOpening()
{
	UpdateSelectableEvents();
}

void SActorIOActionListViewRow::OnEventComboBoxSelectionChanged(FName InName, ESelectInfo::Type InSelectType)
{
	if (InSelectType != ESelectInfo::Direct)
	{
		const FScopedTransaction Transaction(LOCTEXT("ModifyActorIOAction", "Modify ActorIO Action"));
		ActionPtr->Modify();

		UActorIOComponent* ActionOwner = ActionPtr->GetOwnerIOComponent();
		ActionOwner->Modify();

		if (InName == FName(TEXT("<Clear>")))
		{
			InName = NAME_None;
		}

		ActionPtr->EventId = InName;

		//Refresh();
	}
}

FString SActorIOActionListViewRow::OnGetTargetActorPath() const
{
	return ActionPtr->TargetActor.GetPathName();
}

void SActorIOActionListViewRow::OnTargetActorChanged(const FAssetData& InAssetData)
{
	const FScopedTransaction Transaction(LOCTEXT("ModifyActorIOAction", "Modify ActorIO Action"));
	ActionPtr->Modify();

	UActorIOComponent* ActionOwner = ActionPtr->GetOwnerIOComponent();
	ActionOwner->Modify();

	ActionPtr->TargetActor = Cast<AActor>(InAssetData.GetAsset());
	ActionPtr->FunctionId = NAME_None;
	ActionPtr->FunctionArguments = FString();

	UpdateSelectableFunctions();
}

TSharedRef<SWidget> SActorIOActionListViewRow::OnGenerateFunctionComboBoxWidget(FName InName) const
{
	return SNew(STextBlock)
		.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont")) // PropertyEditorConstants::PropertyFontStyle
		.Text(GetFunctionDisplayName(InName))
		.ToolTipText(GetFunctionTooltipText(InName));
}

void SActorIOActionListViewRow::OnFunctionComboBoxOpening()
{
	UpdateSelectableFunctions();
}

void SActorIOActionListViewRow::OnFunctionComboBoxSelectionChanged(FName InName, ESelectInfo::Type InSelectType)
{
	if (InSelectType != ESelectInfo::Direct)
	{
		const FScopedTransaction Transaction(LOCTEXT("ModifyActorIOAction", "Modify ActorIO Action"));
		ActionPtr->Modify();

		UActorIOComponent* ActionOwner = ActionPtr->GetOwnerIOComponent();
		ActionOwner->Modify();

		if (InName == FName(TEXT("<Clear>")))
		{
			InName = NAME_None;
		}

		ActionPtr->FunctionId = InName;
		ActionPtr->FunctionArguments = FString();

		//Refresh();
	}
}

void SActorIOActionListViewRow::OnFunctionParametersChanged(const FText& InText, ETextCommit::Type InCommitType)
{
	const FScopedTransaction Transaction(LOCTEXT("ModifyActorIOAction", "Modify ActorIO Action"));
	ActionPtr->Modify();

	UActorIOComponent* ActionOwner = ActionPtr->GetOwnerIOComponent();
	ActionOwner->Modify();

	ActionPtr->FunctionArguments = InText.ToString();
}

float SActorIOActionListViewRow::OnGetActionDelay() const
{
	return ActionPtr->Delay;
}

void SActorIOActionListViewRow::OnActionDelayChanged(float InValue, ETextCommit::Type InCommitType)
{
	const FScopedTransaction Transaction(LOCTEXT("ModifyActorIOAction", "Modify ActorIO Action"));
	ActionPtr->Modify();

	UActorIOComponent* ActionOwner = ActionPtr->GetOwnerIOComponent();
	ActionOwner->Modify();

	ActionPtr->Delay = InValue;
}

ECheckBoxState SActorIOActionListViewRow::IsExecuteOnlyOnceChecked() const
{
	return ActionPtr->bExecuteOnlyOnce ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SActorIOActionListViewRow::OnExecuteOnlyOnceChecked(ECheckBoxState InState)
{
	const FScopedTransaction Transaction(LOCTEXT("ModifyActorIOAction", "Modify ActorIO Action"));
	ActionPtr->Modify();

	UActorIOComponent* ActionOwner = ActionPtr->GetOwnerIOComponent();
	ActionOwner->Modify();

	ActionPtr->bExecuteOnlyOnce = InState == ECheckBoxState::Checked;
}

FReply SActorIOActionListViewRow::OnClick_RemoveAction()
{
	const FScopedTransaction Transaction(LOCTEXT("RemoveActorIOAction", "Remove ActorIO Action"));

	UActorIOComponent* ActionOwner = ActionPtr->GetOwnerIOComponent();
	ActionOwner->Modify();
	ActionOwner->RemoveAction(ActionPtr.Get());

	FActorIOEditor& ActorIOEditorModule = FModuleManager::GetModuleChecked<FActorIOEditor>("ActorIOEditor");
	ActorIOEditorModule.UpdateEditorWindow();

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE

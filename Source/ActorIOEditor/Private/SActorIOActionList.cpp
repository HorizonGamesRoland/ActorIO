// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "SActorIOActionList.h"
#include "SActorIOEditor.h"
#include "ActorIOComponent.h"
#include "ActorIOEditor.h"
#include "ActorIOEditorStyle.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "PropertyCustomizationHelpers.h"
#include "Misc/Optional.h"

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
	bViewInputActions = InArgs._ViewInputActions;

    SListView::Construct
	(
        SListView::FArguments()
		.ListViewStyle(&FActorIOEditorStyle::Get().GetWidgetStyle<FTableViewStyle>("ActionListView"))
        .ListItemsSource(&ActionListItems)
		.SelectionMode(ESelectionMode::Single)
        .OnGenerateRow(this, &SActorIOActionListView::OnGenerateRowItem)
        .HeaderRow
        (
            SNew(SHeaderRow)
			+ SHeaderRow::Column(ColumnId::ActionType)
			.DefaultLabel(FText::GetEmpty())
			.FixedWidth(30.0f)
			[
				SNew(SBox)
				.HeightOverride(FActorIOEditorStyle::Get().GetFloat("ActionListView.HeaderRowHeight"))
			]

			+ SHeaderRow::Column(ColumnId::Caller)
			.DefaultLabel(LOCTEXT("ActionListView.ColumnCaller", "Caller"))
			.DefaultTooltip(LOCTEXT("ActionListView.ColumnCallerTooltip", "The actor that is calling this action."))
			.FillWidth(this, &SActorIOActionListView::OnGetColumnWidth, ColumnId::Caller)
			.OnWidthChanged(this, &SActorIOActionListView::OnColumnWidthChanged, ColumnId::Caller)
			.HeaderContentPadding(FMargin(5.0f, 0.0f, 0.0f, 0.0f))
			.ShouldGenerateWidget(this, &SActorIOActionListView::IsViewingInputActions)

            +SHeaderRow::Column(ColumnId::Event)
            .DefaultLabel(LOCTEXT("ActionListView.ColumnEvent", "Event"))
			.DefaultTooltip(LOCTEXT("ActionListView.ColumnEventTooltip", "The event that the action is binding to."))
			.FillWidth(this, &SActorIOActionListView::OnGetColumnWidth, ColumnId::Event)
			.OnWidthChanged(this, &SActorIOActionListView::OnColumnWidthChanged, ColumnId::Event)
			.HeaderContentPadding(FMargin(5.0f, 0.0f, 0.0f, 0.0f))

            + SHeaderRow::Column(ColumnId::Target)
            .DefaultLabel(LOCTEXT("ActionListView.ColumnTarget", "Target"))
			.DefaultTooltip(LOCTEXT("ActionListView.ColumnTargetTooltip", "The actor to call the designated function on."))
			.FillWidth(this, &SActorIOActionListView::OnGetColumnWidth, ColumnId::Target)
			.OnWidthChanged(this, &SActorIOActionListView::OnColumnWidthChanged, ColumnId::Target)
			.HeaderContentPadding(FMargin(5.0f, 0.0f, 0.0f, 0.0f))
			.ShouldGenerateWidget(this, &SActorIOActionListView::IsViewingOutputActions)

            + SHeaderRow::Column(ColumnId::Action)
            .DefaultLabel(LOCTEXT("ActionListView.ColumnAction", "Action"))
			.DefaultTooltip(LOCTEXT("ActionListView.ColumnActionTooltip", "The function that is called on the target actor when executing the action."))
			.FillWidth(this, &SActorIOActionListView::OnGetColumnWidth, ColumnId::Action)
			.OnWidthChanged(this, &SActorIOActionListView::OnColumnWidthChanged, ColumnId::Action)
			.HeaderContentPadding(FMargin(5.0f, 0.0f, 0.0f, 0.0f))

            + SHeaderRow::Column(ColumnId::Parameter)
            .DefaultLabel(LOCTEXT("ActionListView.ColumnParameter", "Parameter"))
			.DefaultTooltip(LOCTEXT("ActionListView.ColumnParameterTooltip", "Parameters that are sent with the function."))
			.FillWidth(this, &SActorIOActionListView::OnGetColumnWidth, ColumnId::Parameter)
			.OnWidthChanged(this, &SActorIOActionListView::OnColumnWidthChanged, ColumnId::Parameter)
			.HeaderContentPadding(FMargin(5.0f, 0.0f, 0.0f, 0.0f))

            + SHeaderRow::Column(ColumnId::Delay)
            .DefaultLabel(LOCTEXT("ActionListView.ColumnDelay", "Delay"))
			.DefaultTooltip(LOCTEXT("ActionListView.ColumnDelayTooltip", "Time before the function is called on the target actor, after the action is executed."))
			.FillWidth(this, &SActorIOActionListView::OnGetColumnWidth, ColumnId::Delay)
			.OnWidthChanged(this, &SActorIOActionListView::OnColumnWidthChanged, ColumnId::Delay)
			.HeaderContentPadding(FMargin(5.0f, 0.0f, 0.0f, 0.0f))

            + SHeaderRow::Column(ColumnId::OnlyOnce)
            .DefaultLabel(LOCTEXT("ActionListView.ColumnOnce", "Once?"))
			.DefaultTooltip(LOCTEXT("ActionListView.ColumnOnceTooltip", "Whether the action can only be executed once."))
			.FillWidth(this, &SActorIOActionListView::OnGetColumnWidth, ColumnId::OnlyOnce)
			.OnWidthChanged(this, &SActorIOActionListView::OnColumnWidthChanged, ColumnId::OnlyOnce)
			.HeaderContentPadding(FMargin(5.0f, 0.0f, 0.0f, 0.0f))
        )
    );

    Refresh();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SActorIOActionListView::Refresh()
{
	ActionListItems.Reset();

	FActorIOEditor& ActorIOEditor = FActorIOEditor::Get();
	AActor* SelectedActor = ActorIOEditor.GetSelectedActor();

	if (bViewInputActions)
	{
		ActionListItems = IActorIO::GetInputActionsForObject(SelectedActor);
	}
	else
	{
		ActionListItems = IActorIO::GetOutputActionsForObject(SelectedActor);
	}

	RebuildList();
}

TSharedRef<ITableRow> SActorIOActionListView::OnGenerateRowItem(UActorIOAction* Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	// Figure out if this is the last row so that we can add extra padding at the bottom.
	// I couldn't find a better solution for this.
	bool bIsLastItem = false;
	if (Item && ActionListItems.Num() > 0)
	{
		bIsLastItem = Item == ActionListItems.Last();
	}

    return SNew(SActorIOActionListViewRow, OwnerTable, Item)
		.IsInputAction(bViewInputActions)
		.IsLastItemInList(bIsLastItem);
}

float SActorIOActionListView::OnGetColumnWidth(const FName InColumnName) const
{
	FString PropertyName = TEXT("ActionListView");
	PropertyName += bViewInputActions ? TEXT(".InputColumnWidth.") : TEXT(".OutputColumnWidth.");
	PropertyName += InColumnName.ToString();

	return FActorIOEditorStyle::Get().GetFloat(FName(PropertyName));
}

void SActorIOActionListView::OnColumnWidthChanged(const float InSize, const FName InColumnName)
{
	FString PropertyName = TEXT("ActionListView");
	PropertyName += bViewInputActions ? TEXT(".InputColumnWidth.") : TEXT(".OutputColumnWidth.");
	PropertyName += InColumnName.ToString();

	FActorIOEditorStyle::GetMutableStyle()->Set(FName(PropertyName), InSize);
}


//=======================================================
//~ Begin SActorIOActionListViewRow
//=======================================================

FName SActorIOActionListViewRow::NAME_ClearComboBox = FName(TEXT("<Clear>"));

SLATE_IMPLEMENT_WIDGET(SActorIOActionListViewRow)
void SActorIOActionListViewRow::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

void SActorIOActionListViewRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, UActorIOAction* InActionPtr)
{
	ActionPtr = InActionPtr;
	bIsInputAction = InArgs._IsInputAction;

	UpdateSelectableEvents();
	UpdateSelectableFunctions();

	const float ActionSpacing = FActorIOEditorStyle::Get().GetFloat("ActionListView.ActionSpacing");
	FTableRowArgs RowArgs = FTableRowArgs()
		.ShowSelection(false)
		.OnDragDetected(this, &SActorIOActionListViewRow::HandleDragDetected)
		.OnCanAcceptDrop(this, &SActorIOActionListViewRow::HandleCanAcceptDrop)
		.OnAcceptDrop(this, &SActorIOActionListViewRow::HandleAcceptDrop)
		.Padding(FMargin(0.0f, ActionSpacing, 0.0f, InArgs._IsLastItemInList ? ActionSpacing : 0.0f));

	FSuperRowType::Construct(RowArgs, InOwnerTableView);
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
TSharedRef<SWidget> SActorIOActionListViewRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	TSharedRef<SBox> OutWidget = SNew(SBox)
		.HeightOverride(FActorIOEditorStyle::Get().GetFloat("ActionListView.ActionHeight"));

	if (!ActionPtr)
	{
		// Do nothing if the action is invalid somehow.
		return OutWidget;
	}
	else if (ColumnName == ColumnId::ActionType)
	{
		OutWidget->SetHAlign(HAlign_Center);
		OutWidget->SetVAlign(VAlign_Center);
		OutWidget->SetContent
		(
			SNew(SImage)
			.Image(OnGetActionIcon())
		);
	}
	else if (ColumnName == ColumnId::Caller)
	{
		OutWidget->SetPadding(FMargin(0.0f, 0.0f, 2.0f, 0.0f));
		OutWidget->SetContent
		(
			SNew(SEditableTextBox)
			.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
			.Text(this, &SActorIOActionListViewRow::OnGetCallerNameText)
			.ToolTipText(this, &SActorIOActionListViewRow::OnGetCallerTooltipText)
			.IsEnabled(false)
		);
	}
	else if (ColumnName == ColumnId::Event)
	{
		OutWidget->SetContent
		(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(SWidgetSwitcher)
				.WidgetIndex(bIsInputAction ? 1 : 0)
				+ SWidgetSwitcher::Slot()
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
						.ToolTip(GetEventTooltip(ActionPtr->EventId))
						.ColorAndOpacity(GetEventDisplayColor(ActionPtr->EventId))
					]
				]
				+ SWidgetSwitcher::Slot()
				[
					SNew(SEditableTextBox)
					.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
					.Text(GetEventDisplayName(ActionPtr->EventId))
					.ToolTip(GetEventTooltip(ActionPtr->EventId))
					.ForegroundColor(GetEventDisplayColor(ActionPtr->EventId))
					.IsEnabled(false)
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
			.IsEnabled(!bIsInputAction)
		);
	}
	else if (ColumnName == ColumnId::Action)
	{
		OutWidget->SetPadding(FMargin(0.0f, 0.0f, 1.0f, 0.0f));
		OutWidget->SetContent
		(
			SNew(SWidgetSwitcher)
			.WidgetIndex(bIsInputAction ? 1 : 0)
			+ SWidgetSwitcher::Slot()
			[
				SNew(SComboBox<FName>)
				.OptionsSource(&SelectableFunctionIds)
				.OnGenerateWidget(this, &SActorIOActionListViewRow::OnGenerateFunctionComboBoxWidget)
				.OnComboBoxOpening(this, &SActorIOActionListViewRow::OnFunctionComboBoxOpening)
				.OnSelectionChanged(this, &SActorIOActionListViewRow::OnFunctionComboBoxSelectionChanged)
				[
					SAssignNew(FunctionText, STextBlock)
					.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
					.Text(GetFunctionDisplayName(ActionPtr->FunctionId))
					.ToolTip(GetFunctionTooltip(ActionPtr->FunctionId))
					.ColorAndOpacity(GetFunctionDisplayColor(ActionPtr->FunctionId))
				]
			]
			+ SWidgetSwitcher::Slot()
			[
				SNew(SEditableTextBox)
				.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
				.Text(GetFunctionDisplayName(ActionPtr->FunctionId))
				.ToolTip(GetFunctionTooltip(ActionPtr->FunctionId))
				.ForegroundColor(GetFunctionDisplayColor(ActionPtr->FunctionId))
				.IsEnabled(false)
			]
		);
	}
	else if (ColumnName == ColumnId::Parameter)
	{
		OutWidget->SetPadding(FMargin(1.0f, 0.0f, 1.0f, 0.0f));
		OutWidget->SetContent
		(
			SNew(SEditableTextBox)
			.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
			.Text(FText::FromString(ActionPtr->FunctionArguments))
			.OnTextCommitted(this, &SActorIOActionListViewRow::OnFunctionArgumentsChanged)
			.IsEnabled(!bIsInputAction)
		);
	}
	else if (ColumnName == ColumnId::Delay)
	{
		OutWidget->SetPadding(FMargin(1.0f, 0.0f, 1.0f, 0.0f));
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
			.IsEnabled(!bIsInputAction)
		);
	}
	else if (ColumnName == ColumnId::OnlyOnce)
	{
		OutWidget->SetPadding(FMargin(3.0f, 0.0f, 0.0f, 0.0f));
		OutWidget->SetContent
		(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SCheckBox)
				.IsChecked(this, &SActorIOActionListViewRow::IsExecuteOnlyOnceChecked)
				.OnCheckStateChanged(this, &SActorIOActionListViewRow::OnExecuteOnlyOnceChecked)
				.IsEnabled(!bIsInputAction)
			]
			+ SHorizontalBox::Slot()
			[
				SNew(SSpacer)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 2.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(FActorIOEditorStyle::Get().GetFloat("ActionListView.ActionHeight"))
				[
					SNew(SButton)
					.ButtonStyle(&FActorIOEditorStyle::Get().GetWidgetStyle<FButtonStyle>("ImageButton"))
					.ToolTipText(OnGetRemoveOrViewTooltip())
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.ContentPadding(2.0f)
					.OnClicked(this, &SActorIOActionListViewRow::OnClick_RemoveOrViewAction)
					[
						SNew(SImage)
						.Image(OnGetRemoveOrViewIcon())
						.Visibility(EVisibility::HitTestInvisible)
					]
				]
			]
		);
	}

	return OutWidget;
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedPtr<SActorIOActionListView> SActorIOActionListViewRow::GetOwnerActionListView() const
{
	if (OwnerTablePtr.IsValid())
	{
		TSharedRef<SActorIOActionListView> ActionListView = StaticCastSharedRef<SActorIOActionListView>(OwnerTablePtr.Pin()->AsWidget());
		return ActionListView.ToSharedPtr();
	}

	return nullptr;
}

const FSlateBrush* SActorIOActionListViewRow::OnGetActionIcon() const
{
	const FName BrushName = bIsInputAction ? TEXT("Action.InputIcon") : TEXT("Action.OutputIcon");
	return FActorIOEditorStyle::Get().GetBrush(BrushName);
}

FText SActorIOActionListViewRow::OnGetCallerNameText() const
{
	const AActor* ActionOwner = ActionPtr->GetOwnerActor();
	if (IsValid(ActionOwner))
	{
		return FText::FromString(ActionOwner->GetActorNameOrLabel());
	}

	return FText::FromString(TEXT("None"));
}

FText SActorIOActionListViewRow::OnGetCallerTooltipText() const
{
	const AActor* ActionOwner = ActionPtr->GetOwnerActor();
	if (IsValid(ActionOwner))
	{
		const FString ActorPath = ActionOwner->GetPathName();
		return FText::FormatOrdered(LOCTEXT("ActionListViewRow.CallerTooltip", "Reference to Actor ID '{0}'"), FText::FromString(ActorPath));
	}

	return FText::GetEmpty();
}

TSharedRef<SWidget> SActorIOActionListViewRow::OnGenerateEventComboBoxWidget(FName InName) const
{
	return SNew(STextBlock)
		.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont")) // PropertyEditorConstants::PropertyFontStyle
		.Text(GetEventDisplayName(InName))
		.ToolTip(GetEventTooltip(InName));
}

void SActorIOActionListViewRow::OnEventComboBoxOpening()
{
	UpdateSelectableEvents();
}

void SActorIOActionListViewRow::OnEventComboBoxSelectionChanged(FName InName, ESelectInfo::Type InSelectType)
{
	if (InName == NAME_ClearComboBox)
	{
		InName = NAME_None;
	}

	EventText->SetText(GetEventDisplayName(InName));
	EventText->SetToolTip(GetEventTooltip(InName));
	EventText->SetColorAndOpacity(GetEventDisplayColor(InName));

	if (InSelectType != ESelectInfo::Direct)
	{
		const FScopedTransaction Transaction(LOCTEXT("ModifyActorIOAction", "Modify ActorIO Action"));
		ActionPtr->Modify();

		ActionPtr->EventId = InName;
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

	AActor* NewTarget = Cast<AActor>(InAssetData.GetAsset());
	ActionPtr->TargetActor = NewTarget;

	// Add an I/O component to the selected actor.
	// This is always needed for rendering logic connection lines.
	if (IsValid(NewTarget))
	{
		UActorIOComponent* TargetIOComponent = NewTarget->GetComponentByClass<UActorIOComponent>();
		if (!TargetIOComponent)
		{
			FActorIOEditor& ActorIOEditor = FActorIOEditor::Get();
			ActorIOEditor.AddIOComponentToActor(NewTarget, false);
		}
	}

	GetOwnerActionListView()->Refresh();
}

TSharedRef<SWidget> SActorIOActionListViewRow::OnGenerateFunctionComboBoxWidget(FName InName) const
{
	return SNew(STextBlock)
		.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont")) // PropertyEditorConstants::PropertyFontStyle
		.Text(GetFunctionDisplayName(InName))
		.ToolTip(GetFunctionTooltip(InName));
}

void SActorIOActionListViewRow::OnFunctionComboBoxOpening()
{
	UpdateSelectableFunctions();
}

void SActorIOActionListViewRow::OnFunctionComboBoxSelectionChanged(FName InName, ESelectInfo::Type InSelectType)
{
	if (InName == NAME_ClearComboBox)
	{
		InName = NAME_None;
	}

	FunctionText->SetText(GetFunctionDisplayName(InName));
	FunctionText->SetToolTip(GetFunctionTooltip(InName));
	FunctionText->SetColorAndOpacity(GetFunctionDisplayColor(InName));

	if (InSelectType != ESelectInfo::Direct)
	{
		const FScopedTransaction Transaction(LOCTEXT("ModifyActorIOAction", "Modify ActorIO Action"));
		ActionPtr->Modify();

		ActionPtr->FunctionId = InName;
		ActionPtr->FunctionArguments = FString();

		// Need to refresh to get function arguments widget to update.
		GetOwnerActionListView()->Refresh();
	}
}

void SActorIOActionListViewRow::OnFunctionArgumentsChanged(const FText& InText, ETextCommit::Type InCommitType)
{
	const FScopedTransaction Transaction(LOCTEXT("ModifyActorIOAction", "Modify ActorIO Action"));
	ActionPtr->Modify();

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

	ActionPtr->bExecuteOnlyOnce = InState == ECheckBoxState::Checked;
}

const FSlateBrush* SActorIOActionListViewRow::OnGetRemoveOrViewIcon() const
{
	const FName BrushName = bIsInputAction ? TEXT("Icons.Find") : TEXT("Icons.Delete");
	return FAppStyle::GetBrush(BrushName);
}

FText SActorIOActionListViewRow::OnGetRemoveOrViewTooltip() const
{
	if (!bIsInputAction)
	{
		return LOCTEXT("RemoveActionTooltip", "Delete action");
	}
	else
	{
		return LOCTEXT("ViewActionTooltip", "Select the actor that calls this action");
	}
}

FReply SActorIOActionListViewRow::OnClick_RemoveOrViewAction()
{
	if (!bIsInputAction)
	{
		const FScopedTransaction Transaction(LOCTEXT("RemoveActorIOAction", "Remove ActorIO Action"));
		ActionPtr->Modify();

		UActorIOComponent* ActionOwner = ActionPtr->GetOwnerIOComponent();
		ActionOwner->Modify();
		ActionOwner->RemoveAction(ActionPtr);

		FActorIOEditor& ActorIOEditor = FActorIOEditor::Get();
		ActorIOEditor.UpdateEditorWindow();
	}
	else
	{
		AActor* OwnerActor = ActionPtr->GetOwnerActor();
		if (GEditor && IsValid(OwnerActor))
		{
			// The transaction here is created with a unique context 'ViewIOAction'.
			// The editor module uses this to react to undo/redo of this specific transaction to revert 'bViewInputActions' param.
			// @see FActorIOEditor::PostUndo
			const FScopedTransaction Transaction(TEXT("ViewIOAction"), LOCTEXT("ViewIOAction", "View ActorIO Action"), nullptr);

			// Need to clear selection first, otherwise we are just adding to the selection.
			GEditor->SelectNone(false, true);
			GEditor->SelectActor(OwnerActor, true, true);

			FActorIOEditor& ActorIOEditor = FActorIOEditor::Get();
			SActorIOEditor* EditorWidget = ActorIOEditor.GetEditorWindow();
			EditorWidget->SetViewInputActions(false);
		}
	}

	return FReply::Handled();
}

void SActorIOActionListViewRow::UpdateSelectableEvents()
{
	SelectableEventIds.Reset();
	SelectableEventIds.Add(NAME_ClearComboBox);

	ValidEvents = IActorIO::GetEventsForObject(ActionPtr->GetOwnerActor());
	for (const FActorIOEvent& IOEvent : ValidEvents.EventRegistry)
	{
		SelectableEventIds.Emplace(IOEvent.EventId);
	}
}

void SActorIOActionListViewRow::UpdateSelectableFunctions()
{
	SelectableFunctionIds.Reset();
	SelectableFunctionIds.Add(NAME_ClearComboBox);

	ValidFunctions = IActorIO::GetFunctionsForObject(ActionPtr->TargetActor);
	for (const FActorIOFunction& IOFunction : ValidFunctions.FunctionRegistry)
	{
		SelectableFunctionIds.Emplace(IOFunction.FunctionId);
	}
}

FText SActorIOActionListViewRow::GetEventDisplayName(FName InEventId) const
{
	const FActorIOEvent* TargetEvent = ValidEvents.GetEvent(InEventId);
	if (TargetEvent && !TargetEvent->DisplayName.IsEmpty())
	{
		return TargetEvent->DisplayName;
	}

	return FText::FromName(InEventId);
}

FSlateColor SActorIOActionListViewRow::GetEventDisplayColor(FName InEventId) const
{
	if (InEventId == NAME_None)
	{
		// Also accept 'None' as valid because we only want to highlight outdated events/functions.
		return FSlateColor::UseForeground();
	}

	const FActorIOEvent* TargetEvent = ValidEvents.GetEvent(InEventId);
	return TargetEvent ? FSlateColor::UseForeground() : FStyleColors::Error;
}

TSharedPtr<SActorIOTooltip> SActorIOActionListViewRow::GetEventTooltip(FName InEventId) const
{
	FText TooltipText = FText::GetEmpty();

	const FActorIOEvent* TargetEvent = ValidEvents.GetEvent(InEventId);
	if (TargetEvent)
	{
		TooltipText = TargetEvent->TooltipText;
	}

	TSharedRef<SActorIOTooltip> TooltipWidget = SNew(SActorIOTooltip)
		.RegistryId(InEventId)
		.Description(TooltipText);

	return TooltipWidget.ToSharedPtr();
}

FText SActorIOActionListViewRow::GetFunctionDisplayName(FName InFunctionId) const
{
	const FActorIOFunction* TargetFunction = ValidFunctions.GetFunction(InFunctionId);
	if (TargetFunction && !TargetFunction->DisplayName.IsEmpty())
	{
		return TargetFunction->DisplayName;
	}

	return FText::FromName(InFunctionId);
}

FSlateColor SActorIOActionListViewRow::GetFunctionDisplayColor(FName InFunctionId) const
{
	if (InFunctionId == NAME_None)
	{
		// Also accept 'None' as valid because we only want to highlight outdated events/functions.
		return FSlateColor::UseForeground();
	}

	const FActorIOFunction* TargetFunction = ValidFunctions.GetFunction(InFunctionId);
	return TargetFunction ? FSlateColor::UseForeground() : FStyleColors::Error;
}

TSharedPtr<SActorIOTooltip> SActorIOActionListViewRow::GetFunctionTooltip(FName InFunctionId) const
{
	FText TooltipText = FText::GetEmpty();

	const FActorIOFunction* TargetFunction = ValidFunctions.GetFunction(InFunctionId);
	if (TargetFunction)
	{
		TooltipText = TargetFunction->TooltipText;
	}

	TSharedRef<SActorIOTooltip> TooltipWidget = SNew(SActorIOTooltip)
		.RegistryId(InFunctionId)
		.Description(TooltipText);

	return TooltipWidget.ToSharedPtr();
}

FReply SActorIOActionListViewRow::HandleDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && !bIsInputAction)
	{
		TSharedRef<FActorIOActionDragDropOp> DragDropOp = MakeShared<FActorIOActionDragDropOp>();
		DragDropOp->Element = ActionPtr;

		return FReply::Handled().BeginDragDrop(DragDropOp);
	}

	return FReply::Unhandled();
}

TOptional<EItemDropZone> SActorIOActionListViewRow::HandleCanAcceptDrop(const FDragDropEvent& DragDropEvent, EItemDropZone DropZone, UActorIOAction* TargetItem)
{
	// Similar implementation to array properties in the editor.
	// @see SDetailSingleItemRow::OnArrayCanAcceptDrop

	const FGeometry& Geometry = GetTickSpaceGeometry();
	const float LocalPointerY = Geometry.AbsoluteToLocal(DragDropEvent.GetScreenSpacePosition()).Y;
	const EItemDropZone OverrideDropZone = LocalPointerY < Geometry.GetLocalSize().Y * 0.5f ? EItemDropZone::AboveItem : EItemDropZone::BelowItem;
	return OverrideDropZone;
}

FReply SActorIOActionListViewRow::HandleAcceptDrop(const FDragDropEvent& DragDropEvent, EItemDropZone DropZone, UActorIOAction* TargetItem)
{
	const TSharedPtr<FActorIOActionDragDropOp> DragDropOp = DragDropEvent.GetOperationAs<FActorIOActionDragDropOp>();
	if (!DragDropOp.IsValid())
	{
		return FReply::Unhandled();
	}

	UActorIOComponent* IOComponent = TargetItem ? TargetItem->GetOwnerIOComponent() : nullptr;
	if (!IsValid(IOComponent))
	{
		return FReply::Unhandled();
	}

	int32 FromIdx = IOComponent->GetActions().IndexOfByKey(DragDropOp->Element);
	int32 ToIdx = IOComponent->GetActions().IndexOfByKey(TargetItem);
	if (FromIdx == INDEX_NONE || ToIdx == INDEX_NONE)
	{
		return FReply::Unhandled();
	}

	const FScopedTransaction Transaction(LOCTEXT("MoveActorIOAction", "Move ActorIO Action"));
	IOComponent->Modify();
	IOComponent->MoveAction(FromIdx, ToIdx);

	// Need to refresh since the order of actions changed.
	GetOwnerActionListView()->Refresh();

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE

// Copyright 2025 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

class UActorIOAction;

/**
 * Columns used by the action list.
 * Each column must have a matching entry in FActorIOEditorStyle::SetupActionListColumnSizes!
 */
namespace ColumnId
{
    const FName ActionType = FName(TEXT("ActionType"));
    const FName Caller = FName(TEXT("Caller"));
    const FName Event = FName(TEXT("Event"));
    const FName Target = FName(TEXT("Target"));
    const FName Action = FName(TEXT("Action"));
    const FName Parameter = FName(TEXT("Parameter"));
    const FName Delay = FName(TEXT("Delay"));
    const FName OnlyOnce = FName(TEXT("OnlyOnce"));
}

/**
 * Widget rendering a list of I/O actions found for an actor.
 */
class SActorIOActionListView : public SListView<TWeakObjectPtr<UActorIOAction>>
{
    SLATE_DECLARE_WIDGET(SActorIOActionListView, SListView<TWeakObjectPtr<UActorIOAction>>)

public:

    SLATE_BEGIN_ARGS(SActorIOActionListView)
        : _ViewInputActions(false)
    {}

    /** Whether to show input actions. If false, output actions are shown. */
    SLATE_ARGUMENT(bool, ViewInputActions)

    SLATE_END_ARGS()

    /** Widget constructor. */
    void Construct(const FArguments& InArgs);

    /**
     * Updates the widget to reflect the current state.
     * Used when the widget structure doesn't need to change, just the displayed values.
     */
    void Refresh();

    /** @return Whether the list is currently displaying input actions. */
    bool IsViewingInputActions() const { return bViewInputActions; }

    /** @return Whether the list is currently displaying output actions. */
    bool IsViewingOutputActions() const { return !bViewInputActions; }

protected:

    /** List of I/O actions displayed in the action list. */
    TArray<TWeakObjectPtr<UActorIOAction>> ActionListItems;

    /** Whether the list shows input actions. If false, output actions are shown. */
    bool bViewInputActions;

protected:

    /** Called when a new row is being added to the action list. */
    TSharedRef<ITableRow> OnGenerateRowItem(TWeakObjectPtr<UActorIOAction> Item, const TSharedRef<STableViewBase>& OwnerTable);

    /** @return Width of the given column. */
    float OnGetColumnWidth(const FName InColumnName) const;

    /** Called when a column is resized. */
    void OnColumnWidthChanged(const float InSize, const FName InColumnName);
};


/**
 * Widget of a single row in the action list.
 * This is basically the UI representation of an I/O action.
 */
class SActorIOActionListViewRow : public SMultiColumnTableRow<TWeakObjectPtr<UActorIOAction>>
{
    SLATE_DECLARE_WIDGET(SActorIOActionListViewRow, SMultiColumnTableRow<TWeakObjectPtr<UActorIOAction>>)

public:

    SLATE_BEGIN_ARGS(SActorIOActionListViewRow)
        : _IsInputAction(false)
        , _IsLastItemInList(false)
    {}

    /** Whether this action is an input action of the actor. If false, it's an output action. */
    SLATE_ARGUMENT(bool, IsInputAction)
    /** Whether this action is the last element in the list. */
    SLATE_ARGUMENT(bool, IsLastItemInList)

    SLATE_END_ARGS()

    /** Widget constructor. */
    void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, TWeakObjectPtr<UActorIOAction> InActionPtr);

    /**
     * Generate the contents of the given column.
     * Called when the row is being constructed.
     */
    virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

    /** @return The action list that owns this row. */
    TSharedPtr<SActorIOActionListView> GetOwnerActionListView() const;

protected:

    /** Weak reference to the I/O action that this widget represents. */
    TWeakObjectPtr<UActorIOAction> ActionPtr;

    /** Whether this is an input action on the actor. If false, it's an output action. */
    bool bIsInputAction;

    /** Cached list of registered I/O events found on the action's owning actor. */
    FActorIOEventList ValidEvents;

    /** Cached list of registered I/O functions found on the action's target actor. */
    FActorIOFunctionList ValidFunctions;

    /**
     * Name to be used as the "clear" action in the dropdown menu when selecting an I/O event or function.
     * Since combo boxes do not support setting the selection to none, we use this instead.
     */
    static FName NAME_ClearComboBox;

    /**
     * List of I/O event ids that are selectable in the event combo box.
     * Always contains the same ids found in ValidEvents above.
     */
    TArray<FName> SelectableEventIds;

    /**
     * List of I/O function ids that are selectable in the function combo box.
     * Always contains the same ids found in ValidFunctions above.
     */
    TArray<FName> SelectableFunctionIds;

    /** Text block of the event combo box. */
    TSharedPtr<class STextBlock> EventText;

    /** Text block of the function combo box. */
    TSharedPtr<class STextBlock> FunctionText;

protected:

    /** @return Icon to be displayed at the very beginning of the row. */
    const FSlateBrush* OnGetActionIcon() const;

    /** @return Name of the actor that owns the action. Only used when showing input actions. */
    FText OnGetCallerNameText() const;

    /** Called when generating an entry for the event combo box. */
    TSharedRef<SWidget> OnGenerateEventComboBoxWidget(FName InName) const;

    /** Called before the event combo box is opened. */
    void OnEventComboBoxOpening();

    /** Called when the selection changes for the event combo box. */
    void OnEventComboBoxSelectionChanged(FName InName, ESelectInfo::Type InSelectType);

    /** @return Path to the target actor. */
    FString OnGetTargetActorPath() const;

    /** Called when the selected target actor is changed. */
    void OnTargetActorChanged(const FAssetData& InAssetData);

    /** Called when generating an entry for the function combo box. */
    TSharedRef<SWidget> OnGenerateFunctionComboBoxWidget(FName InName) const;

    /** Called before the function combo box is opened. */
    void OnFunctionComboBoxOpening();

    /** Called when the selection changes for the function combo box. */
    void OnFunctionComboBoxSelectionChanged(FName InName, ESelectInfo::Type InSelectType);

    /** Called when the function parameters are changed in the text box. */
    void OnFunctionParametersChanged(const FText& InText, ETextCommit::Type InCommitType);

    /** @return Delay of the action. */
    float OnGetActionDelay() const;

    /** Called when the value changes in the delay spin box. */
    void OnActionDelayChanged(float InValue, ETextCommit::Type InCommitType);

    /** @return True if the execute only once toggle should be checked. */
    ECheckBoxState IsExecuteOnlyOnceChecked() const;

    /** Called when execute only once is toggled. */
    void OnExecuteOnlyOnceChecked(ECheckBoxState InState);

    /** @return Icon to be displayed in the button at the very end of the row. */
    const FSlateBrush* OnGetRemoveOrViewIcon() const;

    /** @return Tooltip text to be used for the button at the very end of the row. */
    FText OnGetRemoveOrViewTooltip() const;

    /** Called when the button at the very end of the row is clicked. */
    FReply OnClick_RemoveOrViewAction();

protected:

    /** Updates the list of selectable events. */
    void UpdateSelectableEvents();

    /** Updates the list of selectable functions. */
    void UpdateSelectableFunctions();

    /** Finds the display name of the given I/O event. */
    FText GetEventDisplayName(FName InEventId) const;

    /** Finds the tooltipp text of the given I/O event. */
    FText GetEventTooltipText(FName InEventId) const;

    /** @return Color based on whether the given I/O event is valid or not. */
    FSlateColor GetEventTextColor(FName InEventId) const;

    /** Finds the display name of the given I/O function. */
    FText GetFunctionDisplayName(FName InFunctionId) const;

    /** Finds the tooltipp text of the given I/O function. */
    FText GetFunctionTooltipText(FName InFunctionId) const;

    /** @return Color based on whether the given I/O function is valid or not. */
    FSlateColor GetFunctionTextColor(FName InFunctionId) const;
};

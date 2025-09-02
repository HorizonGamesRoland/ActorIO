// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "ActorIOAction.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

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
class ACTORIOEDITOR_API SActorIOActionListView : public SListView<TWeakObjectPtr<UActorIOAction>>
{
    SLATE_DECLARE_WIDGET(SActorIOActionListView, SListView<TWeakObjectPtr<UActorIOAction>>)

public:

    SLATE_BEGIN_ARGS(SActorIOActionListView)
    {}

    /** Reference to the I/O editor widget that owns this action list. */
    SLATE_ARGUMENT(TWeakPtr<class SActorIOEditor>, IOEditor);

    SLATE_END_ARGS()

    /** Widget constructor. */
    void Construct(const FArguments& InArgs);

    /**
     * Updates the widget to reflect the current state.
     * Used when the widget structure doesn't need to change, just the displayed values.
     */
    void Refresh();

    /** @return Whether the list is currently displaying input actions. */
    bool IsViewingInputActions() const;

    /** @return Whether the list is currently displaying output actions. */
    bool IsViewingOutputActions() const;

    /**
     * Spawns the params viewer widget that list the parameters of the given UFunction.
     * The widget will be placed above the given parent widget.
     * Used by the arguments edit box of action list entries.
     */
    void ShowParamsViewer(UFunction* InFunction, const TSharedRef<SWidget>& InParentWidget);

    /** Closes the params viewer widget if there is one. */
    void CloseParamsViewer();

    /** Changes the highlighted param index in the params viewer widget if there is one. */
    void UpdateParamsViewer(int32 InHighlightedParamIdx);

    /**
     * Detect if the editor should be refreshed (e.g. action's target actor became unloaded).
     * Called every frame during I/O editor widget tick.
     */
    bool TickAutoRefreshRequired() const;

protected:

    /** Reference to the I/O editor widget that owns this action list. */
    TWeakPtr<class SActorIOEditor> IOEditor;

    /** Popup menu of the params viewer that is visible while editing action params. */
    TSharedPtr<class IMenu> ParamsViewerMenu;

    /** Reference to the params viewer widget inside the popup menu. */
    TSharedPtr<class SActorIOParamsViewer> ParamsViewerWidget;

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
class ACTORIOEDITOR_API SActorIOActionListViewRow : public SMultiColumnTableRow<TWeakObjectPtr<UActorIOAction>>
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

    /** @return Weak reference to the action that this row represents. */
    const TWeakObjectPtr<UActorIOAction>& GetActionPtr() const { return ActionPtr; }

    /** @return The action list that owns this row. */
    TSharedPtr<SActorIOActionListView> GetOwnerActionListView() const;

    /**
     * Get the cached state of whether the action's target actor is pending or not.
     * Only set during construction, so it can become out of date.
     * If out of date, the action list forces a refresh during SActorIOActionListView::TickAutoRefreshRequired.
     */
    bool GetTargetActorIsPending() const { return bIsTargetActorPending; }

protected:

    /** The I/O action that this widget represents. */
    TWeakObjectPtr<UActorIOAction> ActionPtr;

    /** Whether this is an input action on the actor. If false, it's an output action. */
    bool bIsInputAction;

    /** Whether the action's target actor is pending or not. */
    bool bIsTargetActorPending;

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

    /** Actor picker widget. */
    TSharedPtr<class SObjectPropertyEntryBox> ActorPicker;

    /** Editable text box where function arguments are inputted. */
    TSharedPtr<class SMultiLineEditableTextBox> ArgumentsBox;

    /** Error reporting widget of the function arguments edit box. */
    TSharedPtr<class SActorIOErrorText> ArgumentsErrorText;

protected:

    /** @return Icon to be displayed at the very beginning of the row. */
    const FSlateBrush* OnGetActionIcon() const;

    /** @return Name of the actor that owns the action. Only used when showing input actions. */
    FText OnGetCallerNameText() const;

    /** @return Reference path of the actor that owns the action. Only used when showing input actions. */
    FText OnGetCallerTooltipText() const;

    /** Called when generating an entry for the event combo box. */
    TSharedRef<SWidget> OnGenerateEventComboBoxWidget(FName InName);

    /** Called before the event combo box is opened. */
    void OnEventComboBoxOpening();

    /** Called when the selection changes for the event combo box. */
    void OnEventComboBoxSelectionChanged(FName InName, ESelectInfo::Type InSelectType);

    /** @return Path to the target actor. */
    FString OnGetTargetActorPath() const;

    /** Called when the selected target actor is changed. */
    void OnTargetActorChanged(const FAssetData& InAssetData);

    /** Called when the selected target actor is invalid, but path info exists so the actor is most likely unloaded. */
    void OnTargetActorIsPending();

    /** Called when generating an entry for the function combo box. */
    TSharedRef<SWidget> OnGenerateFunctionComboBoxWidget(FName InName);

    /** Called before the function combo box is opened. */
    void OnFunctionComboBoxOpening();

    /** Called when the selection changes for the function combo box. */
    void OnFunctionComboBoxSelectionChanged(FName InName, ESelectInfo::Type InSelectType);

    /** Called when the function parameters are changed in the text box. */
    void OnFunctionArgumentsChanged(const FText& InText);

    /** Called when the function parameters are committed in the text box. */
    void OnFunctionArgumentsCommitted(const FText& InText, ETextCommit::Type InCommitType);

    /** Called when the caret is moved inside the function parrameters text box. */
    void OnFunctionArgumentsCursorMoved(const FTextLocation& InTextLocation);

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

    /** @return Color based on whether the given I/O event is valid or not. */
    FSlateColor GetEventDisplayColor(FName InEventId) const;

    /** @return Tooltip widget to use for I/O events. */
    TSharedPtr<class SToolTip> GetEventTooltip(FName InEventId);

    /** Finds the display name of the given I/O function. */
    FText GetFunctionDisplayName(FName InFunctionId) const;

    /** @return Color based on whether the given I/O function is valid or not. */
    FSlateColor GetFunctionDisplayColor(FName InFunctionId) const;

    /** @return Tooltip widget to use for I/O functions. */
    TSharedPtr<class SToolTip> GetFunctionTooltip(FName InFunctionId);

    /** Validate function arguments and push error text to error reporting widget. */
    void UpdateFunctionArgumentsErrorText(const FText& InArguments);

    /** @return Check whether the given function arguments are valid or not. */
    bool ValidateFunctionArguments(const FText& InText, FText& OutError);

public:

    //~ Begin SWidget Interface
    virtual void OnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent) override;
    //~ End SWidget Interface

    //~ Begin Drag & Drop
    FReply HandleDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
    TOptional<EItemDropZone> HandleCanAcceptDrop(const FDragDropEvent& DragDropEvent, EItemDropZone DropZone, TWeakObjectPtr<UActorIOAction> TargetItem);
    FReply HandleAcceptDrop(const FDragDropEvent& DragDropEvent, EItemDropZone DropZone, TWeakObjectPtr<UActorIOAction> TargetItem);
    //~ End Drag & Drop
};


/**
 * Drag & Drop operation for SActorIOActionListViewRow.
 */
class ACTORIOEDITOR_API FActorIOActionDragDropOp : public FDragDropOperation
{
public:

    DRAG_DROP_OPERATOR_TYPE(FActorIOActionDragDropOp, FDragDropOperation)

    /** The IO action that is being dragged. */
    TWeakObjectPtr<UActorIOAction> Element;
};


/**
 * Class for iterating through all childs of a widget, including childrens of child widgets.
 * Iteration continues until the iterator func returns false, or we run out of widgets.
 */
class ACTORIOEDITOR_API FActorIOChildWidgetIterator
{
public:

    /**
     * Callback function when iterating over a widget.
     * Return false to stop the iterator.
     */
    typedef TFunction<bool(SWidget&)> TWidgetIterationFunc;

    /** Constructor. */
    FActorIOChildWidgetIterator(SWidget& InParent, TWidgetIterationFunc InIterationFunc);
    FActorIOChildWidgetIterator() = delete;

private:

    /** Recursively iterate over all child widgets of the given widget. */
    bool Advance(SWidget& InWidget);

    /** Function to call whenever we iterate over a widget. */
    TWidgetIterationFunc IterationFunc;
};

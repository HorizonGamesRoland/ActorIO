// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

class UActorIOAction;
struct FActorIOEvent;
struct FActorIOFunction;

namespace ColumnId
{
    const FName ActionType = FName(TEXT("ActionType"));
    const FName Event = FName(TEXT("Event"));
    const FName Target = FName(TEXT("Target"));
    const FName Action = FName(TEXT("Action"));
    const FName Parameter = FName(TEXT("Parameter"));
    const FName Delay = FName(TEXT("Delay"));
    const FName OnlyOnce = FName(TEXT("OnlyOnce"));
}

enum class EActorIOActionListViewMode : uint8
{
    Inputs,

    Outputs
};

class SActorIOActionListView : public SListView<TWeakObjectPtr<UActorIOAction>>
{
    SLATE_DECLARE_WIDGET(SActorIOActionListView, SListView<TWeakObjectPtr<UActorIOAction>>)

public:

    SLATE_BEGIN_ARGS(SActorIOActionListView)
        : _ViewMode(EActorIOActionListViewMode::Outputs)
    {}

    SLATE_ATTRIBUTE(EActorIOActionListViewMode, ViewMode)

    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    void Refresh();

    void SetViewMode(EActorIOActionListViewMode InMode);

    EActorIOActionListViewMode GetViewMode() const { return ActionListViewMode; }

protected:

    TArray<TWeakObjectPtr<UActorIOAction>> ActionListItems;

    EActorIOActionListViewMode ActionListViewMode;

protected:

    TSharedRef<ITableRow> OnGenerateRowItem(TWeakObjectPtr<UActorIOAction> Item, const TSharedRef<STableViewBase>& OwnerTable);
};



class SActorIOActionListViewRow : public SMultiColumnTableRow<TWeakObjectPtr<UActorIOAction>>
{
    SLATE_DECLARE_WIDGET(SActorIOActionListViewRow, SMultiColumnTableRow<TWeakObjectPtr<UActorIOAction>>)

public:

    SLATE_BEGIN_ARGS(SActorIOActionListViewRow)
    {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, TWeakObjectPtr<UActorIOAction> InActionPtr);

    virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

protected:

    TWeakObjectPtr<UActorIOAction> ActionPtr;

    TArray<FActorIOEvent> ValidEvents;

    TArray<FActorIOFunction> ValidFunctions;

    TArray<FName> SelectableEventIds;

    TArray<FName> SelectableFunctionIds;

    TSharedPtr<class STextBlock> EventText;

    TSharedPtr<class STextBlock> FunctionText;

protected:

    TSharedRef<SWidget> OnGenerateEventComboBoxWidget(FName InName) const;

    void OnEventComboBoxOpening();

    void OnEventComboBoxSelectionChanged(FName InName, ESelectInfo::Type InSelectType);

    FString OnGetTargetActorPath() const;

    void OnTargetActorChanged(const FAssetData& InAssetData);

    TSharedRef<SWidget> OnGenerateFunctionComboBoxWidget(FName InName) const;

    void OnFunctionComboBoxOpening();

    void OnFunctionComboBoxSelectionChanged(FName InName, ESelectInfo::Type InSelectType);

    void OnFunctionParametersChanged(const FText& InText, ETextCommit::Type InCommitType);

    float OnGetActionDelay() const;

    void OnActionDelayChanged(float InValue, ETextCommit::Type InCommitType);

    ECheckBoxState IsExecuteOnlyOnceChecked() const;

    void OnExecuteOnlyOnceChecked(ECheckBoxState InState);

    FReply OnClick_RemoveAction();

protected:

    void UpdateSelectableEvents();

    void UpdateSelectableFunctions();

    FText GetEventDisplayName(FName InEventId) const;

    FText GetEventTooltipText(FName InEventId) const;

    FSlateColor GetEventTextColor(FName InEventId) const;

    FText GetFunctionDisplayName(FName InFunctionId) const;

    FText GetFunctionTooltipText(FName InFunctionId) const;

    FSlateColor GetFunctionTextColor(FName InFunctionId) const;
};

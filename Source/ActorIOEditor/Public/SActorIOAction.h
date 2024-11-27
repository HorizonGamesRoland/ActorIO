// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class UActorIOAction;
struct FActorIOEvent;
struct FActorIOFunction;

class SActorIOAction : public SCompoundWidget
{
    SLATE_DECLARE_WIDGET(SActorIOAction, SCompoundWidget)

public:

    SLATE_BEGIN_ARGS(SActorIOAction)
        : _Action(nullptr)
        , _PropertySizes(TArray<float>())
    {}
        
    SLATE_ARGUMENT(TWeakObjectPtr<UActorIOAction>, Action)
    SLATE_ARGUMENT(TArray<float>, PropertySizes)

    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    virtual void Refresh();

    void SetPropertySize(int32 SlotIdx, float InSize);

protected:

    TWeakObjectPtr<UActorIOAction> Action;

    TSharedPtr<class SSplitter> PropertySplitter;

    TArray<FActorIOEvent> ValidEvents;

    TArray<FActorIOFunction> ValidFunctions;

    TArray<FName> SelectableEventIds;

    TArray<FName> SelectableFunctionIds;

protected:

    virtual void InitializeAction() {}

    void UpdateSelectableEvents();

    void UpdateSelectableFunctions();

    FText GetEventDisplayName(FName InEventId) const;

    FText GetEventTooltipText(FName InEventId) const;

    FColor GetEventTextColor(FName InEventId) const;

    FText GetFunctionDisplayName(FName InFunctionId) const;

    FText GetFunctionTooltipText(FName InFunctionId) const;

    FColor GetFunctionTextColor(FName InFunctionId) const;
};


class SActorOutputAction : public SActorIOAction
{
    SLATE_DECLARE_WIDGET(SActorOutputAction, SActorIOAction)

public:

    SLATE_BEGIN_ARGS(SActorOutputAction)
        : _Action(nullptr)
        , _PropertySizes(TArray<float>())
    {}
        
    SLATE_ARGUMENT(TWeakObjectPtr<UActorIOAction>, Action)
    SLATE_ARGUMENT(TArray<float>, PropertySizes)

    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    virtual void InitializeAction() override;
    virtual void Refresh() override;

protected:

    TSharedPtr<class STextBlock> EventText;

    TSharedPtr<class STextBlock> FunctionText;

protected:

    TSharedRef<SWidget> OnGenerateEventComboBoxWidget(FName InName);

    TSharedRef<SWidget> OnGenerateFunctionComboBoxWidget(FName InName);

    void OnEventChanged(FName InName, ESelectInfo::Type InSelectType);

    void OnTargetActorChanged(const FAssetData& InAssetData);

    FString GetTargetActorPath() const;

    void OnTargetFunctionChanged(FName InName, ESelectInfo::Type InSelectType);

    void OnFunctionArgumentsChanged(const FText& InText, ETextCommit::Type InCommitType);

    FReply OnClick_RemoveAction();
};



class SActorInputAction : public SActorIOAction
{
    SLATE_DECLARE_WIDGET(SActorInputAction, SActorIOAction)

public:

    SLATE_BEGIN_ARGS(SActorInputAction)
        : _Action(nullptr)
        , _PropertySizes(TArray<float>())
    {}
    
    SLATE_ARGUMENT(TWeakObjectPtr<UActorIOAction>, Action)
    SLATE_ARGUMENT(TArray<float>, PropertySizes)

    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    virtual void InitializeAction() override;
    virtual void Refresh() override;

protected:

    TSharedPtr<class SEditableTextBox> CallerTextBox;

    TSharedPtr<class SEditableTextBox> EventTextBox;

    TSharedPtr<class SEditableTextBox> FunctionTextBox;

    TSharedPtr<class SEditableTextBox> FunctionArgumentsTextBox;
};

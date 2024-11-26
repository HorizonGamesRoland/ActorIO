// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class UActorIOAction;
struct FActorIOEvent;
struct FActorIOFunction;

class SActorIOAction : public SCompoundWidget
{
public:

    SLATE_BEGIN_ARGS(SActorIOAction)
    {}
        
        SLATE_ARGUMENT(UActorIOAction*, Action)
        SLATE_ARGUMENT(TArray<float>, PropertySizes)

    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    void Refresh();

    void SetPropertySize(int32 SlotIdx, float InSize);

protected:

    UActorIOAction* Action; // #TODO: Convert to WeakPtr

    TSharedPtr<class SSplitter> PropertySplitter;

    TSharedPtr<class STextBlock> EventText;

    TSharedPtr<class STextBlock> FunctionText;

    TArray<FActorIOEvent> ValidEvents;

    TArray<FActorIOFunction> ValidFunctions;

    TArray<FName> SelectableEventIds;

    TArray<FName> SelectableFunctionIds;

protected:

    TSharedRef<SWidget> OnGenerateEventComboBoxWidget(FName InName);

    void OnEventChanged(FName InName, ESelectInfo::Type InSelectType);

    TSharedRef<SWidget> OnGenerateFunctionComboBoxWidget(FName InName);

    void OnTargetFunctionChanged(FName InName, ESelectInfo::Type InSelectType);

    void OnTargetActorChanged(const FAssetData& InAssetData);

    FString GetTargetActorPath() const;

    void OnFunctionArgumentsChanged(const FText& InText, ETextCommit::Type InCommitType);

    FReply OnClick_RemoveAction();

protected:

    void UpdateSelectableEvents();

    void UpdateSelectableFunctions();

    FText GetEventDisplayName(FName InEventId) const;

    FText GetEventTooltipText(FName InEventId) const;

    FColor GetEventTextColor(FName InEventId) const;

    FText GetFunctionDisplayName(FName InFunctionId) const;

    FText GetFunctionTooltipText(FName InFunctionId) const;

    FColor GetFunctionTextColor(FName InFunctionId) const;
};

// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class UActorIOComponent;
struct FActorIOEvent;
struct FActorIOFunction;
struct FActorIOAction;

class SActorIOAction : public SCompoundWidget
{
public:

    SLATE_BEGIN_ARGS(SActorIOAction)
    {}
        
        SLATE_ARGUMENT(TObjectPtr<UActorIOComponent>, IOComponent)
        SLATE_ARGUMENT(int32, ActionIdx)

    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    void RebuildWidget();

protected:

    TObjectPtr<UActorIOComponent> IOComponent;

    int32 ActionIdx;

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

    FActorIOAction& GetAction() const;

    FText GetEventDisplayName(FName InEventId) const;

    FText GetEventTooltipText(FName InEventId) const;

    FText GetFunctionDisplayName(FName InFunctionId) const;

    FText GetFunctionTooltipText(FName InFunctionId) const;

    bool DoesEventExist(FName InEventId) const;

    bool DoesFunctionExist(FName InFunctionId) const;
};

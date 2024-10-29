// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class UActorIOComponent;
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

    TArray<FName> SelectableEvents;

    TArray<FName> SelectableFunctions;

protected:

    TSharedRef<SWidget> OnGenerateComboBoxWidget(FName InName);

    void OnEventChanged(FName InName, ESelectInfo::Type InSelectType);

    void OnTargetFunctionChanged(FName InName, ESelectInfo::Type InSelectType);

    void OnTargetActorChanged(const FAssetData& InAssetData);

    FString GetTargetActorPath() const;

    FReply OnClick_RemoveAction();

protected:

    void UpdateSelectableEvents();

    void UpdateSelectableFunctions();

    FActorIOAction& GetAction() const;
};

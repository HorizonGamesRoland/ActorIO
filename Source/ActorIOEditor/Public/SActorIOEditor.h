// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SActorIOEditor : public SCompoundWidget
{
public:

    SLATE_BEGIN_ARGS(SActorIOEditor)
    {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    void RebuildWidget();

protected:

    TSharedPtr<class STextBlock> ActorNameText;

    TSharedPtr<class STextBlock> OutputsButtonText;

    TSharedPtr<class STextBlock> InputsButtonText;

    TSharedPtr<class SBox> ActionPanel;

    TSharedPtr<class SSplitter> ActionSplitter;

    TSharedPtr<class SVerticalBox> ActionList;

protected:

    bool bViewOutputs;

protected:

    const TSharedRef<SWidget> ConstructOutputsTab();

    FReply OnClick_Outputs();

    FReply OnClick_Inputs();

    FReply OnClick_NewAction();
};

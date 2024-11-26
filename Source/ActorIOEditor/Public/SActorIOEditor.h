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

    void Refresh();

protected:

    TSharedPtr<class STextBlock> SelectedActorText;

    TSharedPtr<class STextBlock> OutputsButtonText;

    TSharedPtr<class STextBlock> InputsButtonText;

    TSharedPtr<class SBox> ActionPanel;

    TSharedPtr<class SActorOutputsTab> OutputsTab;

    TSharedPtr<class SActorInputsTab> InputsTab;

    bool bViewOutputs;

protected:

    FReply OnClick_Outputs();

    FReply OnClick_Inputs();

    FReply OnClick_NewAction();
};

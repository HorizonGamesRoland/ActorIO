// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SActorIOEditor : public SCompoundWidget
{
    SLATE_DECLARE_WIDGET(SActorIOEditor, SCompoundWidget)

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

    TSharedPtr<class SPositiveActionButton> NewActionButton;

    TSharedPtr<class SBorder> ActionListContainer;

    TSharedPtr<class SActorIOActionListView> ActionListView;

    bool bViewInputActions;

    bool bActionListNeedsRegenerate;

protected:

    ECheckBoxState IsOutputsButtonChecked() const;

    void OnOutputsButtonChecked(ECheckBoxState InState);

    ECheckBoxState IsInputsButtonChecked() const;

    void OnInputsButtonChecked(ECheckBoxState InState);

    FReply OnClick_NewAction();
};

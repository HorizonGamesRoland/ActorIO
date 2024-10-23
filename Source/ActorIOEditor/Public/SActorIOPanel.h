// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Docking/SDockTab.h"

class FActorIOEditor;

class SActorIOPanel : public SCompoundWidget
{
public:

    SLATE_BEGIN_ARGS(SActorIOPanel)
    {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    void RebuildWidget();

protected:

    bool bViewOutputs;

    TSharedPtr<class STextBlock> ActorNameText;

    TSharedPtr<class SVerticalBox> ActionsBox;

protected:

    const TSharedRef<SWidget> ConstructMenuPanel(const FArguments& InArgs);

    const TSharedRef<SWidget> ConstructDetailsPanel(const FArguments& InArgs);

    FReply OnClick_Outputs();

    FReply OnClick_Inputs();

    FReply OnClick_NewAction();
};

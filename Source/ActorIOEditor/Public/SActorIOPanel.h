// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Docking/SDockTab.h"

class UActorIOComponent;
struct FActorIOAction;

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

    TArray<FName> SelectableEvents;

protected:

    const TSharedRef<SWidget> ConstructHeaderRow();

    const TSharedRef<SWidget> ConstructActionRow(UActorIOComponent* InActorIOComponent, int32 ActionIdx);

    FReply OnClick_Outputs();

    FReply OnClick_Inputs();

    FReply OnClick_NewAction();

    void ModifyAction_Event(UActorIOComponent* InActorIOComponent, int32 ActionIdx, FName InNewEvent);
};

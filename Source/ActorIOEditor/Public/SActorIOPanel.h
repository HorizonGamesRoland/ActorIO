// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Docking/SDockTab.h"

class AActor;

class SActorIOPanel : public SCompoundWidget
{
public:

    SLATE_BEGIN_ARGS(SActorIOPanel)
    {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    void RebuildFromState(AActor* InActor);

protected:

    const TSharedRef<SWidget> ConstructMenuPanel(const FArguments& InArgs);

    const TSharedRef<SWidget> ConstructDetailsPanel(const FArguments& InArgs);

    bool ValidateElements() const;

    TSharedPtr<class STextBlock> ActorNameText;
};

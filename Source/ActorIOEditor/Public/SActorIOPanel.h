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

    bool ValidateElements() const;

protected:

    TSharedPtr<class STextBlock> ActorNameText;
};

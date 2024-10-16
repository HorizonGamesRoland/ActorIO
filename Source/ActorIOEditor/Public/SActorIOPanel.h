// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Docking/SDockTab.h"

class ActorIOEditor;

class SActorIOPanel : public SCompoundWidget
{
public:

    SLATE_BEGIN_ARGS(SActorIOPanel)
    {}
    SLATE_ARGUMENT(TWeakPtr<ActorIOEditor>, Tool)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

protected:

    TWeakPtr<ActorIOEditor> MyEditor;
};

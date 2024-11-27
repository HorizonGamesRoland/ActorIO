// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SActorIOTab : public SCompoundWidget
{
    SLATE_DECLARE_WIDGET(SActorIOTab, SCompoundWidget)

public:

    SLATE_BEGIN_ARGS(SActorIOTab)
    {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    virtual void Refresh() {}

protected:

    TSharedPtr<class SSplitter> ActionPropertySplitter;

    TArray<float> ActionPropertySizes;

    TSharedPtr<class SVerticalBox> ActionList;

protected:

    virtual void InitializeHeaderRow() {}

    void AddPropertyHeader(const FText& InPropertyName, const FMargin& InPadding);

    void OnActionPropertyResized(int32 InSlotIndex, float InSize);
};


class SActorOutputsTab : public SActorIOTab
{
    SLATE_DECLARE_WIDGET(SActorOutputsTab, SActorIOTab)

public:

    SLATE_BEGIN_ARGS(SActorOutputsTab)
    {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    virtual void InitializeHeaderRow() override;
    virtual void Refresh() override;
};


class SActorInputsTab : public SActorIOTab
{
    SLATE_DECLARE_WIDGET(SActorInputsTab, SActorIOTab)

public:

    SLATE_BEGIN_ARGS(SActorInputsTab)
    {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    virtual void InitializeHeaderRow() override;
    virtual void Refresh() override;
};

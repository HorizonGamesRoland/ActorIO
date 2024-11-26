// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SActorIOTab : public SCompoundWidget
{
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
public:

    SLATE_BEGIN_ARGS(SActorIOTab)
    {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    virtual void InitializeHeaderRow() override;
    virtual void Refresh() override;
};


class SActorInputsTab : public SActorIOTab
{
public:

    SLATE_BEGIN_ARGS(SActorIOTab)
    {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    virtual void InitializeHeaderRow() override;
    virtual void Refresh() override;
};

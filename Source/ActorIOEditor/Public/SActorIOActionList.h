// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SActorIOActionList : public SCompoundWidget
{
    SLATE_DECLARE_WIDGET(SActorIOActionList, SCompoundWidget)

public:

    SLATE_BEGIN_ARGS(SActorIOActionList)
    {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    virtual void Refresh() {}

protected:

    TSharedPtr<class SBox> PropertyHeaderContainer;

    TSharedPtr<class SSplitter> PropertyHeaderSplitter;

    TArray<float> PropertyHeaderSizes;

    TSharedPtr<class SVerticalBox> ActionList;

protected:

    virtual void InitializeHeaderRow() {}

    void AddPropertyHeader(const FText& InPropertyName, float InSizeValue, const FMargin& InPadding);

    void OnPropertyHeaderResized(int32 InSlotIndex, float InSize);

    void OnScrollBarVisibilityChanged(EVisibility InVisibility);
};


class SActorOutputList : public SActorIOActionList
{
    SLATE_DECLARE_WIDGET(SActorOutputList, SActorIOActionList)

public:

    SLATE_BEGIN_ARGS(SActorOutputList)
    {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    virtual void InitializeHeaderRow() override;
    virtual void Refresh() override;
};


class SActorInputList : public SActorIOActionList
{
    SLATE_DECLARE_WIDGET(SActorInputList, SActorIOActionList)

public:

    SLATE_BEGIN_ARGS(SActorInputList)
    {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    virtual void InitializeHeaderRow() override;
    virtual void Refresh() override;
};

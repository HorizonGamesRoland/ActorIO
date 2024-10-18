// Copyright 2024 Horizon Games. All Rights Reserved.

#include "SActorIOPanel.h"
#include "GameFramework/Actor.h"

#define LOCTEXT_NAMESPACE "ActorIOPanel"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SActorIOPanel::Construct(const FArguments& InArgs)
{
    ChildSlot
    [
        SNew(SBox)
        .Padding(5.0f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .HAlign(HAlign_Left)
            .AutoWidth()
            [
                SNew(SBox)
                .WidthOverride(250)
                .Padding(0.0f, 0.0f, 5.0f, 0.0f)
                [
                    ConstructMenuPanel(InArgs)
                ]
            ]
            + SHorizontalBox::Slot()
            [
                ConstructDetailsPanel(InArgs)
            ]
        ]
    ];
}

const TSharedRef<SWidget> SActorIOPanel::ConstructMenuPanel(const FArguments& InArgs)
{
    return SNew(SBorder)
        .BorderBackgroundColor(FColor(192, 192, 192, 255))
        .Padding(5.0f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(5.0f, 2.0f, 5.0f, 5.0f)
            [
                SAssignNew(ActorNameText, STextBlock)
            ]
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0.0f, 0.0f, 0.0f, 2.0f)
            [
                SNew(SButton)
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Center)
                .ContentPadding(10.0f)
                .Text(LOCTEXT("Outputs", "Outputs (0)"))
            ]
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0.0f, 0.0f, 0.0f, 2.0f)
            [
                SNew(SButton)
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Center)
                .ContentPadding(10.0f)
                .Text(LOCTEXT("Inputs", "Inputs (0)"))
            ]
            + SVerticalBox::Slot()
            .FillHeight(1.0)
            [
                SNew(SSpacer)
            ]
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SButton)
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Center)
                .ContentPadding(10.0f)
                .Text(LOCTEXT("NewAction", "+ New Action"))
            ]
        ];
}

const TSharedRef<SWidget> SActorIOPanel::ConstructDetailsPanel(const FArguments& InArgs)
{
    return SNew(SBorder)
        .BorderBackgroundColor(FColor(192, 192, 192, 255))
        .Padding(5.0f)
        [
            SNew(SBox)
        ];
}

void SActorIOPanel::RebuildFromState(AActor* InActor)
{
    if (!ValidateElements())
    {
        // Do nothing if any of our widget elements are invalid.
        // This should never happen.
        return;
    }

    FString ActorName = InActor ? InActor->GetActorNameOrLabel() : TEXT("None");
    ActorNameText->SetText(FText::Format(LOCTEXT("SelectedActorName", "Actor: {0}"), FText::FromString(ActorName)));
}

bool SActorIOPanel::ValidateElements() const
{
    return ActorNameText.IsValid();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE

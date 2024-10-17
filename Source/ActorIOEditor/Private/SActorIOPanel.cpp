// Copyright 2024 Horizon Games. All Rights Reserved.

#include "SActorIOPanel.h"
#include "GameFramework/Actor.h"

#define LOCTEXT_NAMESPACE "ActorIOPanel"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SActorIOPanel::Construct(const FArguments& InArgs)
{
    ChildSlot
    [
        SNew(SScrollBox)
        + SScrollBox::Slot()
        .VAlign(VAlign_Top)
        .Padding(5)
        [
            SNew(SBorder)
            .BorderBackgroundColor(FColor(192, 192, 192, 255))
            .Padding(15.0f)
            [
                SAssignNew(ActorNameText, STextBlock)
            ]
        ]
    ];
}

void SActorIOPanel::RebuildFromState(AActor* InActor)
{
    if (!ValidateElements())
    {
        return;
    }

    FString ActorName = InActor ? InActor->GetActorNameOrLabel() : TEXT("None");
    ActorNameText->SetText(FText::Format(LOCTEXT("SelectedActorName", "Selected: {0}"), FText::FromString(ActorName)));
}

bool SActorIOPanel::ValidateElements() const
{
    return ActorNameText.IsValid();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE

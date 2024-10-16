// Copyright 2024 Horizon Games. All Rights Reserved.

#include "SActorIOPanel.h"

void SActorIOPanel::Construct(const FArguments& InArgs)
{
    MyEditor = InArgs._Tool;
    if (MyEditor.IsValid())
    {
        // do anything you need from tool object
    }

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
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("This is a tab example.")))
            ]
        ]
    ];
}

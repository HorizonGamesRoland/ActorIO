// Copyright 2024 Horizon Games. All Rights Reserved.

#include "SActorIOEditor.h"
#include "SActorIOAction.h"
#include "ActorIOEditor.h"
#include "ActorIOEditorSubsystem.h"
#include "ActorIOComponent.h"
#include "ActorIOAction.h"
#include "GameFramework/Actor.h"
#include "LevelEditor.h"

#define LOCTEXT_NAMESPACE "ActorIOEditor"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SActorIOEditor::Construct(const FArguments& InArgs)
{
    // Display output actions by default.
    bViewOutputs = true;

    ChildSlot
    [
        SNew(SSplitter)
        + SSplitter::Slot()
        .Value(0.33f)
        [
            // ---------------------------------
            //~ Menu panel with buttons
            // ---------------------------------
            SNew(SBox)
            .Padding(3.0f)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(5.0f, 0.0f, 0.0f, 0.0f)
                [
                    SNew(SBox)
                    .HeightOverride(30.0f)
                    .VAlign(VAlign_Center)
                    [
                        SAssignNew(ActorNameText, STextBlock)
                    ]
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0.0f, 0.0f, 0.0f, 2.0f)
                [
                    SNew(SButton)
                    .HAlign(HAlign_Center)
                    .VAlign(VAlign_Center)
                    .ContentPadding(10.0f)
                    .OnClicked(this, &SActorIOEditor::OnClick_Outputs)
                    [
                        SAssignNew(OutputsButtonText, STextBlock)
                        .Visibility(EVisibility::HitTestInvisible)
                    ]
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0.0f, 0.0f, 0.0f, 2.0f)
                [
                    SNew(SButton)
                    .HAlign(HAlign_Center)
                    .VAlign(VAlign_Center)
                    .ContentPadding(10.0f)
                    .OnClicked(this, &SActorIOEditor::OnClick_Inputs)
                    [
                        SAssignNew(InputsButtonText, STextBlock)
                        .Visibility(EVisibility::HitTestInvisible)
                    ]
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
                    .OnClicked(this, &SActorIOEditor::OnClick_NewAction)
                ]
            ]
        ]
        + SSplitter::Slot()
        [
            // ---------------------------------
            //~ Actions panel
            // ---------------------------------
            SAssignNew(ActionPanel, SBox)
        ]
    ];

    RebuildWidget();
}

void SActorIOEditor::RebuildWidget()
{
    UActorIOEditorSubsystem* ActorIOEditorSubsystem = GEditor->GetEditorSubsystem<UActorIOEditorSubsystem>();
    AActor* SelectedActor = ActorIOEditorSubsystem ? ActorIOEditorSubsystem->GetSelectedActor() : nullptr;
    UActorIOComponent* ActorIOComponent = SelectedActor ? SelectedActor->GetComponentByClass<UActorIOComponent>() : nullptr;

    const FString ActorName = SelectedActor ? SelectedActor->GetActorNameOrLabel() : TEXT("None");
    ActorNameText->SetText(FText::FormatNamed(LOCTEXT("SelectedActorName", "Actor: {Name}"),
        TEXT("Name"), FText::FromString(ActorName)));

    const int32 NumOutputActions = ActorIOComponent ? ActorIOComponent->GetNumActions() : 0;
    OutputsButtonText->SetText(FText::FormatNamed(LOCTEXT("OutputsButton", "Outputs ({Count})"),
        TEXT("Count"), NumOutputActions));

    const int32 NumInputActions = 0;
    InputsButtonText->SetText(FText::FormatNamed(LOCTEXT("InputsButton", "Inputs ({Count})"),
        TEXT("Count"), NumInputActions));

    if (bViewOutputs)
    {
        ActionPanel->SetContent(ConstructOutputsTab());
    }
    else
    {
        ActionPanel->SetContent(SNew(SBox));
    }

    if (ActorIOComponent)
    {
        for (int32 ActionIdx = 0; ActionIdx != ActorIOComponent->GetActions().Num(); ++ActionIdx)
        {
            ActionList->AddSlot()
            .AutoHeight()
            [
                SNew(SActorIOAction)
                .IOComponent(ActorIOComponent)
                .ActionIdx(ActionIdx)
            ];
        }
    }
}

const TSharedRef<SWidget> SActorIOEditor::ConstructOutputsTab()
{
    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SBox)
            .HeightOverride(30.0f)
            [
                SAssignNew(ActionSplitter, SSplitter)
                + SSplitter::Slot()
                [
                    SNew(SBox)
                    .VAlign(VAlign_Center)
                    .Padding(30.0f, 0.0f, 5.0f, 0.0f)
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("Event", "Event:"))
                    ]     
                ]
                + SSplitter::Slot()
                [
                    SNew(SBox)
                    .VAlign(VAlign_Center)
                    .Padding(5.0f, 0.0f)
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("Target", "Target:"))
                    ]
                ]
                + SSplitter::Slot()
                [
                    SNew(SBox)
                    .VAlign(VAlign_Center)
                    .Padding(5.0f, 0.0f)
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("Action", "Action:"))
                    ]
                ]
                + SSplitter::Slot()
                [
                    SNew(SBox)
                    .VAlign(VAlign_Center)
                    .Padding(5.0f, 0.0f)
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("Parameters", "Parameters:"))
                    ]
                ]
            ]
        ]
        + SVerticalBox::Slot()
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::Get().GetBrush("Brushes.Recessed"))
            [
                SAssignNew(ActionList, SVerticalBox)
            ]
        ];
}

FReply SActorIOEditor::OnClick_Outputs()
{
    if (!bViewOutputs)
    {
        bViewOutputs = true;
        RebuildWidget();
    }
    
    return FReply::Handled();
}

FReply SActorIOEditor::OnClick_Inputs()
{
    if (bViewOutputs)
    {
        bViewOutputs = false;
        RebuildWidget();
    }

    return FReply::Handled();
}

FReply SActorIOEditor::OnClick_NewAction()
{
    UActorIOEditorSubsystem* ActorIOEditorSubsystem = GEditor->GetEditorSubsystem<UActorIOEditorSubsystem>();
    AActor* SelectedActor = ActorIOEditorSubsystem->GetSelectedActor();
    if (SelectedActor)
    {
        const FScopedTransaction Transaction(LOCTEXT("AddActorIOAction", "Add ActorIO Action"));

        UActorIOComponent* ActorIOComponent = SelectedActor->GetComponentByClass<UActorIOComponent>();
        if (!ActorIOComponent)
        {
            SelectedActor->Modify();

            ActorIOComponent = NewObject<UActorIOComponent>(SelectedActor, TEXT("ActorIOComponent"), RF_Transactional);
            ActorIOComponent->OnComponentCreated();
            ActorIOComponent->RegisterComponent();

            SelectedActor->AddInstanceComponent(ActorIOComponent);

            FLevelEditorModule& LevelEditor = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
            LevelEditor.BroadcastComponentsEdited();
        }

        if (ActorIOComponent)
        {
            ActorIOComponent->Modify();

            TArray<FActorIOAction>& Actions = ActorIOComponent->GetActions();
            Actions.Emplace();
        }
    }

    RebuildWidget();
    return FReply::Handled();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE

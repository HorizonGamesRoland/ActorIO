// Copyright 2024 Horizon Games. All Rights Reserved.

#include "SActorIOEditor.h"
#include "SActorIOAction.h"
#include "ActorIOEditor.h"
#include "ActorIOEditorSubsystem.h"
#include "ActorIOComponent.h"
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
        .Value(0.4f)
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
                    .Text(LOCTEXT("Outputs", "Outputs (0)"))
                    .OnClicked(this, &SActorIOEditor::OnClick_Outputs)
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
                    .OnClicked(this, &SActorIOEditor::OnClick_Inputs)
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
    if (bViewOutputs)
    {
        ActionPanel->SetContent(ConstructOutputsTab());
    }
    else
    {
        ActionPanel->SetContent(SNew(SBox));
    }

    UActorIOEditorSubsystem* ActorIOEditorSubsystem = GEditor->GetEditorSubsystem<UActorIOEditorSubsystem>();
    AActor* SelectedActor = ActorIOEditorSubsystem->GetSelectedActor();

    FString ActorName = SelectedActor ? SelectedActor->GetActorNameOrLabel() : TEXT("None");
    ActorNameText->SetText(FText::Format(LOCTEXT("SelectedActorName", "Actor: {0}"), FText::FromString(ActorName)));

    UActorIOComponent* ActorIOComponent = SelectedActor ? SelectedActor->GetComponentByClass<UActorIOComponent>() : nullptr;
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
                    .Padding(5.0f, 0.0f)
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
            ]
        ]
        + SVerticalBox::Slot()
        .Padding(3.0f)
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::Get().GetBrush("PlacementBrowser.Asset.Background"))
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

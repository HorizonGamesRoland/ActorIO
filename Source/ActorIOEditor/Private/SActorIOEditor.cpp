// Copyright 2024 Horizon Games. All Rights Reserved.

#include "SActorIOEditor.h"
#include "SActorIOActionList.h"
#include "SActorIOAction.h"
#include "ActorIOEditor.h"
#include "ActorIOEditorSubsystem.h"
#include "ActorIOSystem.h"
#include "ActorIOComponent.h"
#include "ActorIOAction.h"
#include "GameFramework/Actor.h"
#include "LevelEditor.h"

#define LOCTEXT_NAMESPACE "ActorIOEditor"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

SLATE_IMPLEMENT_WIDGET(SActorIOEditor)
void SActorIOEditor::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

void SActorIOEditor::Construct(const FArguments& InArgs)
{
    OutputsTab = SNew(SActorOutputList);
    InputsTab = SNew(SActorInputList);

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
                    .HeightOverride(25.0f)
                    .VAlign(VAlign_Center)
                    [
                        SAssignNew(SelectedActorText, STextBlock)
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
            SNew(SBox)
            .Padding(0.0f, 3.0f, 3.0f, 3.0f)
            [
                // ---------------------------------
                //~ Actions panel
                // ---------------------------------
                SAssignNew(ActionPanel, SBox)
            ]
        ]
    ];

    Refresh();
}

void SActorIOEditor::Refresh()
{
    UActorIOEditorSubsystem* ActorIOEditorSubsystem = GEditor->GetEditorSubsystem<UActorIOEditorSubsystem>();
    AActor* SelectedActor = ActorIOEditorSubsystem ? ActorIOEditorSubsystem->GetSelectedActor() : nullptr;
    UActorIOComponent* ActorIOComponent = SelectedActor ? SelectedActor->GetComponentByClass<UActorIOComponent>() : nullptr;

    const FString ActorName = SelectedActor ? SelectedActor->GetActorNameOrLabel() : TEXT("None");
    SelectedActorText->SetText(FText::FormatNamed(LOCTEXT("SelectedActorName", "Actor: {Name}"),
        TEXT("Name"), FText::FromString(ActorName)));

    const int32 NumOutputActions = ActorIOComponent ? ActorIOComponent->GetNumActions() : 0;
    OutputsButtonText->SetText(FText::FormatNamed(LOCTEXT("OutputsButton", "Outputs ({Count})"),
        TEXT("Count"), NumOutputActions));

    const int32 NumInputActions = UActorIOSystem::GetNumInputActionsForObject(SelectedActor);
    InputsButtonText->SetText(FText::FormatNamed(LOCTEXT("InputsButton", "Inputs ({Count})"),
        TEXT("Count"), NumInputActions));

    if (bViewOutputs)
    {
        ActionPanel->SetContent(OutputsTab.ToSharedRef());
        OutputsTab->Refresh();
    }
    else
    {
        ActionPanel->SetContent(InputsTab.ToSharedRef());
        InputsTab->Refresh();
    }
}

FReply SActorIOEditor::OnClick_Outputs()
{
    if (!bViewOutputs)
    {
        bViewOutputs = true;
        Refresh();
    }
    
    return FReply::Handled();
}

FReply SActorIOEditor::OnClick_Inputs()
{
    if (bViewOutputs)
    {
        bViewOutputs = false;
        Refresh();
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
            ActorIOComponent->CreateNewAction();
        }
    }

    Refresh();
    return FReply::Handled();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE

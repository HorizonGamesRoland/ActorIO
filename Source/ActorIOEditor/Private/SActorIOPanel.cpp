// Copyright 2024 Horizon Games. All Rights Reserved.

#include "SActorIOPanel.h"
#include "ActorIOEditor.h"
#include "ActorIOEditorSubsystem.h"
#include "ActorIOComponent.h"
#include "GameFramework/Actor.h"
#include "LevelEditor.h"

#define LOCTEXT_NAMESPACE "ActorIOPanel"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SActorIOPanel::Construct(const FArguments& InArgs)
{
    // By default display the outputs tab.
    bViewOutputs = true;

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
                .OnClicked(this, &SActorIOPanel::OnClick_Outputs)
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
                .OnClicked(this, &SActorIOPanel::OnClick_Inputs)
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
                .OnClicked(this, &SActorIOPanel::OnClick_NewAction)
            ]
        ];
}

const TSharedRef<SWidget> SActorIOPanel::ConstructDetailsPanel(const FArguments& InArgs)
{
    return SNew(SBorder)
        .BorderBackgroundColor(FColor(192, 192, 192, 255))
        .Padding(5.0f)
        [
            SAssignNew(ActionsBox, SVerticalBox)
        ];
}

void SActorIOPanel::RebuildWidget()
{
    ActionsBox->ClearChildren();

    UActorIOEditorSubsystem* ActorIOEditorSubsystem = GEditor->GetEditorSubsystem<UActorIOEditorSubsystem>();
    AActor* SelectedActor = ActorIOEditorSubsystem->GetSelectedActor();

    FString ActorName = SelectedActor ? SelectedActor->GetActorNameOrLabel() : TEXT("None");
    ActorNameText->SetText(FText::Format(LOCTEXT("SelectedActorName", "Actor: {0}"), FText::FromString(ActorName)));

    UActorIOComponent* ActorIOComponent = SelectedActor ? SelectedActor->GetComponentByClass<UActorIOComponent>() : nullptr;
    if (ActorIOComponent)
    {
        for (int32 ActionIdx = 0; ActionIdx != ActorIOComponent->GetActions().Num(); ++ActionIdx)
        {
            ActionsBox->AddSlot()
            .AutoHeight()
            [
                ConstructActionRow(ActorIOComponent, ActionIdx)
            ];
        }
    }
}

const TSharedRef<SWidget> SActorIOPanel::ConstructActionRow(UActorIOComponent* InActorIOComponent, int32 ActionIdx)
{
    const FActorIOAction& Action = InActorIOComponent->GetActions()[ActionIdx];

    SelectableEvents.Reset();
    SelectableEvents.Add(TEXT("Null"));

    TArray<FActorIOEvent> ValidEvents = InActorIOComponent->GetEvents();
    for (const FActorIOEvent& IOEvent : ValidEvents)
    {
        SelectableEvents.Emplace(IOEvent.EventName);
    }

    return SNew(SBox)
    .HeightOverride(30)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .HAlign(HAlign_Left)
        .AutoWidth()
        [
            SNew(SBox)
            .WidthOverride(200)
            .Padding(0.0f, 0.0f, 0.0f, 5.0f)
            [
                SNew(SComboBox<FName>)
                .OptionsSource(&SelectableEvents)
                .OnGenerateWidget_Lambda([](FName InName)
                {
                    return SNew(STextBlock)
                        .Text(FText::FromName(InName));
                })
                .OnSelectionChanged_Lambda([this, InActorIOComponent, ActionIdx](FName InName, ESelectInfo::Type InSelectType)
                {
                    if (InSelectType != ESelectInfo::Direct)
                    {
                        ModifyAction_Event(InActorIOComponent, ActionIdx, InName);
                    }
                })
                .Content()
                [
                    SNew(STextBlock)
                    .Text(FText::FromName(Action.SourceEvent))
                ]
            ]
        ]
    ];
}

FReply SActorIOPanel::OnClick_Outputs()
{
    if (!bViewOutputs)
    {
        bViewOutputs = true;
        RebuildWidget();
    }
    
    return FReply::Handled();
}

FReply SActorIOPanel::OnClick_Inputs()
{
    if (bViewOutputs)
    {
        bViewOutputs = false;
        RebuildWidget();
    }

    return FReply::Handled();
}

FReply SActorIOPanel::OnClick_NewAction()
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

void SActorIOPanel::ModifyAction_Event(UActorIOComponent* InActorIOComponent, int32 ActionIdx, FName InNewEvent)
{
    if (InActorIOComponent)
    {
        TArray<FActorIOAction>& Actions = InActorIOComponent->GetActions();
        if (Actions.IsValidIndex(ActionIdx))
        {
            if (Actions[ActionIdx].SourceEvent != InNewEvent)
            {
                const FScopedTransaction Transaction(LOCTEXT("ModifyActorIOAction", "Modify ActorIO Action"));
                InActorIOComponent->Modify();

                Actions[ActionIdx].SourceEvent = InNewEvent;
            }
        }
    }

    RebuildWidget();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE

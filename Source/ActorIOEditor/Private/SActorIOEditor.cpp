// Copyright 2024 Horizon Games. All Rights Reserved.

#include "SActorIOEditor.h"
#include "SActorIOActionList.h"
#include "SActorIOAction.h"
#include "ActorIOEditor.h"
#include "ActorIOEditorSubsystem.h"
#include "ActorIOEditorStyle.h"
#include "ActorIOSystem.h"
#include "ActorIOComponent.h"
#include "ActorIOAction.h"
#include "GameFramework/Actor.h"
#include "LevelEditor.h"
#include "SPositiveActionButton.h"
#include "Styling/SlateTypes.h"
#include "Misc/Optional.h"

#define LOCTEXT_NAMESPACE "ActorIOEditor"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

SLATE_IMPLEMENT_WIDGET(SActorIOEditor)
void SActorIOEditor::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

void SActorIOEditor::Construct(const FArguments& InArgs)
{
    OutputActionList = SNew(SActorOutputList);
    InputActionList = SNew(SActorInputList);

    // Display output actions by default.
    bViewOutputs = true;

    ChildSlot
    [
        SNew(SBox)
        .Padding(3.0f)
        [
            SNew(SSplitter)
            .PhysicalSplitterHandleSize(0.0f)
            + SSplitter::Slot()
            .MinSize(70)
            .Value(0.33f)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0.0f, 0.0f, 0.0f, 3.0f)
                [
                    SNew(SBorder)
                    .BorderImage(FActorIOEditorStyle::Get().GetBrush("RoundedHeader"))
                    .Padding(0.0f)
                    [
                        SNew(SBox)
                        .HeightOverride(FActorIOEditorStyle::HeaderRowHeight)
                        .Padding(10.0f, 0.0f)
                        [
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot()
                            .VAlign(VAlign_Center)
                            .AutoWidth()
                            .Padding(0.0f, 0.0f, 5.0f, 0.0f)
                            [
                                SNew(SImage)
                                .Image(FAppStyle::Get().GetBrush("ClassIcon.Actor"))
                            ]
                            + SHorizontalBox::Slot()
                            .VAlign(VAlign_Center)
                            [
                                SAssignNew(SelectedActorText, STextBlock)
                            ]
                        ]
                    ]
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0.0f, 0.0f, 0.0f, 3.0f)
                [
                    SNew(SBox)
                    .HeightOverride(FActorIOEditorStyle::ToolButtonHeight)
                    [
                        SNew(SCheckBox)
                        .Type(ESlateCheckBoxType::ToggleButton)
                        .Style(&FActorIOEditorStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckbox"))
                        .IsChecked(this, &SActorIOEditor::IsOutputsButtonChecked)
                        .OnCheckStateChanged(this, &SActorIOEditor::OnOutputsButtonChecked)
                        [
                            SNew(SBox)
                            .HAlign(HAlign_Center)
                            .VAlign(VAlign_Center)
                            [
                                SAssignNew(OutputsButtonText, STextBlock)
                                .Visibility(EVisibility::HitTestInvisible)
                            ]
                        ]
                    ]
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0.0f, 0.0f, 0.0f, 3.0f)
                [
                    SNew(SBox)
                    .HeightOverride(FActorIOEditorStyle::ToolButtonHeight)
                    [
                        SNew(SCheckBox)
                        .Type(ESlateCheckBoxType::ToggleButton)
                        .Style(&FActorIOEditorStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckbox"))
                        .IsChecked(this, &SActorIOEditor::IsInputsButtonChecked)
                        .OnCheckStateChanged(this, &SActorIOEditor::OnInputsButtonChecked)
                        [
                            SNew(SBox)
                            .HAlign(HAlign_Center)
                            .VAlign(VAlign_Center)
                            [
                                SAssignNew(InputsButtonText, STextBlock)
                                .Visibility(EVisibility::HitTestInvisible)
                            ]
                        ]
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
                    SNew(SBox)
                    .HeightOverride(FActorIOEditorStyle::ToolButtonHeight)
                    [
                        SNew(SPositiveActionButton)
                        .Text(LOCTEXT("NewAction", "New Action"))
                        .OnClicked(this, &SActorIOEditor::OnClick_NewAction)
                    ]
                ]
            ]
            + SSplitter::Slot()
            [
                SAssignNew(ActionPanel, SBox)
                .Padding(3.0f, 0.0f, 0.0f, 0.0f)
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
    SelectedActorText->SetText(FText::FromString(ActorName));

    const int32 NumOutputActions = ActorIOComponent ? ActorIOComponent->GetNumActions() : 0;
    OutputsButtonText->SetText(FText::FormatNamed(LOCTEXT("OutputsButton", "Outputs ({Count})"),
        TEXT("Count"), NumOutputActions));

    const int32 NumInputActions = UActorIOSystem::GetNumInputActionsForObject(SelectedActor);
    InputsButtonText->SetText(FText::FormatNamed(LOCTEXT("InputsButton", "Inputs ({Count})"),
        TEXT("Count"), NumInputActions));

    if (bViewOutputs)
    {
        ActionPanel->SetContent(SNew(SActorOutputListView));
        OutputActionList->Refresh();
    }
    else
    {
        ActionPanel->SetContent(InputActionList.ToSharedRef());
        InputActionList->Refresh();
    }
}

ECheckBoxState SActorIOEditor::IsOutputsButtonChecked() const
{
    return bViewOutputs ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SActorIOEditor::OnOutputsButtonChecked(ECheckBoxState InState)
{
    if (InState == ECheckBoxState::Checked && !bViewOutputs)
    {
        bViewOutputs = true;
        Refresh();
    }
}

ECheckBoxState SActorIOEditor::IsInputsButtonChecked() const
{
    return !bViewOutputs ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SActorIOEditor::OnInputsButtonChecked(ECheckBoxState InState)
{
    if (InState == ECheckBoxState::Checked && bViewOutputs)
    {
        bViewOutputs = false;
        Refresh();
    }
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

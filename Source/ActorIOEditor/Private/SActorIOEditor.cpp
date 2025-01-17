// Copyright 2025 Horizon Games. All Rights Reserved.

#include "SActorIOEditor.h"
#include "SActorIOActionList.h"
#include "ActorIOEditor.h"
#include "ActorIOEditorStyle.h"
#include "ActorIOSystem.h"
#include "ActorIOComponent.h"
#include "ActorIOAction.h"
#include "GameFramework/Actor.h"
#include "SPositiveActionButton.h"
#include "Styling/SlateTypes.h"
#include "Misc/Optional.h"

#define LOCTEXT_NAMESPACE "ActorIOEditor"

SLATE_IMPLEMENT_WIDGET(SActorIOEditor)
void SActorIOEditor::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SActorIOEditor::Construct(const FArguments& InArgs)
{
    bViewInputActions = false;
    bActionListNeedsRegenerate = true;

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
                        SAssignNew(NewActionButton, SPositiveActionButton)
                        .Text(LOCTEXT("NewAction", "New Action"))
                        .OnClicked(this, &SActorIOEditor::OnClick_NewAction)
                    ]
                ]
            ]
            + SSplitter::Slot()
            [
                SNew(SBox)
                .Padding(3.0f, 0.0f, 0.0f, 0.0f)
                [
                    SAssignNew(ActionListContainer, SBorder)
                    .BorderImage(FActorIOEditorStyle::Get().GetBrush("ActionListView.Border"))
                    .Padding(1.0f)
                ]
            ]
        ]
    ];

    Refresh();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SActorIOEditor::Refresh()
{
    FActorIOEditor& ActorIOEditor = FActorIOEditor::Get();
    AActor* SelectedActor = ActorIOEditor.GetSelectedActor();
    UActorIOComponent* ActorIOComponent = SelectedActor ? SelectedActor->GetComponentByClass<UActorIOComponent>() : nullptr;

    const FString ActorName = SelectedActor ? SelectedActor->GetActorNameOrLabel() : TEXT("None");
    SelectedActorText->SetText(FText::FromString(ActorName));

    const int32 NumOutputActions = ActorIOComponent ? ActorIOComponent->GetNumActions() : 0;
    OutputsButtonText->SetText(FText::FormatNamed(LOCTEXT("OutputsButton", "Outputs ({Count})"),
        TEXT("Count"), NumOutputActions));

    const int32 NumInputActions = UActorIOSystem::GetNumInputActionsForObject(SelectedActor);
    InputsButtonText->SetText(FText::FormatNamed(LOCTEXT("InputsButton", "Inputs ({Count})"),
        TEXT("Count"), NumInputActions));

    const bool bCanAddAction = IsValid(SelectedActor) && !bViewInputActions;
    NewActionButton->SetEnabled(bCanAddAction);

    if (bActionListNeedsRegenerate)
    {
        bActionListNeedsRegenerate = false;
        ActionListContainer->SetContent
        (
            SAssignNew(ActionListView, SActorIOActionListView)
            .ViewInputActions(bViewInputActions)
        );
    }
    else
    {
        ActionListView->Refresh();
    }
}

void SActorIOEditor::SetViewInputActions(bool bEnabled, bool bRefresh)
{
    bViewInputActions = bEnabled;
    if (bRefresh)
    {
        bActionListNeedsRegenerate = true;
        Refresh();
    }
}

ECheckBoxState SActorIOEditor::IsOutputsButtonChecked() const
{
    return bViewInputActions ? ECheckBoxState::Unchecked : ECheckBoxState::Checked;
}

void SActorIOEditor::OnOutputsButtonChecked(ECheckBoxState InState)
{
    if (InState == ECheckBoxState::Checked && bViewInputActions)
    {
        bViewInputActions = false;
        bActionListNeedsRegenerate = true;
        Refresh();
    }
}

ECheckBoxState SActorIOEditor::IsInputsButtonChecked() const
{
    return bViewInputActions ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SActorIOEditor::OnInputsButtonChecked(ECheckBoxState InState)
{
    if (InState == ECheckBoxState::Checked && !bViewInputActions)
    {
        bViewInputActions = true;
        bActionListNeedsRegenerate = true;
        Refresh();
    }
}

FReply SActorIOEditor::OnClick_NewAction()
{
    FActorIOEditor& ActorIOEditor = FActorIOEditor::Get();
    AActor* SelectedActor = ActorIOEditor.GetSelectedActor();
    if (IsValid(SelectedActor))
    {
        const FScopedTransaction Transaction(LOCTEXT("AddActorIOAction", "Add ActorIO Action"));

        UActorIOComponent* ActorIOComponent = SelectedActor->GetComponentByClass<UActorIOComponent>();
        if (!ActorIOComponent)
        {
            ActorIOComponent = ActorIOEditor.AddIOComponenToActor(SelectedActor, true);
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

#undef LOCTEXT_NAMESPACE

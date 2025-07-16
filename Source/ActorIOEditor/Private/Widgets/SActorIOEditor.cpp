// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "Widgets/SActorIOEditor.h"
#include "Widgets/SActorIOActionList.h"
#include "ActorIOEditor.h"
#include "ActorIOEditorStyle.h"
#include "ActorIOComponent.h"
#include "ActorIOAction.h"
#include "GameFramework/Actor.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Input/SCheckBox.h"
#include "SPositiveActionButton.h"
#include "Styling/SlateIconFinder.h"
#include "Styling/SlateTypes.h"
#include "ScopedTransaction.h"
#include "Misc/Optional.h"
#include "SlateOptMacros.h"

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
            .Value(0.3f)
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
                        .HeightOverride(FActorIOEditorStyle::Get().GetFloat("ActionListView.HeaderRowHeight"))
                        .Padding(10.0f, 0.0f)
                        [
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot()
                            .VAlign(VAlign_Center)
                            .AutoWidth()
                            .Padding(0.0f, 0.0f, 5.0f, 0.0f)
                            [
                                SAssignNew(SelectedActorIcon, SImage)
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
                    .HeightOverride(FActorIOEditorStyle::Get().GetFloat("ToggleButtonHeight"))
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
                    .HeightOverride(FActorIOEditorStyle::Get().GetFloat("ToggleButtonHeight"))
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
                    .HeightOverride(FActorIOEditorStyle::Get().GetFloat("ToggleButtonHeight"))
                    [
                        SAssignNew(NewActionButton, SPositiveActionButton)
                        .Text(LOCTEXT("NewAction", "New Action"))
                        .ToolTipText(LOCTEXT("NewActionTooltip", "Add a new output action to the selected actor."))
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
    const FString ActorPath = SelectedActor ? SelectedActor->GetPathName() : TEXT("None");
    const FText ActorTooltip = FText::FormatOrdered(LOCTEXT("SelectedActorTooltip", "Reference to Actor ID '{0}'"), FText::FromString(ActorPath));
    SelectedActorText->SetText(FText::FromString(ActorName));
    SelectedActorText->SetToolTipText(ActorTooltip);

    const UClass* ActorClass = SelectedActor ? SelectedActor->GetClass() : AActor::StaticClass();
    const FSlateBrush* ActorIcon = FSlateIconFinder::FindIconBrushForClass(ActorClass, "ClassIcon.Actor");
    SelectedActorIcon->SetImage(ActorIcon);
    SelectedActorIcon->SetToolTipText(ActorTooltip);

    const int32 NumOutputActions = ActorIOComponent ? ActorIOComponent->GetNumActions() : 0;
    OutputsButtonText->SetText(FText::FormatOrdered(LOCTEXT("OutputsButton", "Outputs ({0})"), FText::AsNumber(NumOutputActions)));

    const int32 NumInputActions = IActorIO::GetNumInputActionsForObject(SelectedActor);
    InputsButtonText->SetText(FText::FormatOrdered(LOCTEXT("InputsButton", "Inputs ({0})"), FText::AsNumber(NumInputActions)));

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
            ActorIOComponent = ActorIOEditor.AddIOComponentToActor(SelectedActor, true);
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

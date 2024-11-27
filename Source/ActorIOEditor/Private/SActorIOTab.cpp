// Copyright 2024 Horizon Games. All Rights Reserved.

#include "SActorIOTab.h"
#include "SActorIOAction.h"
#include "ActorIOEditorSubsystem.h"
#include "ActorIOComponent.h"
#include "ActorIOAction.h"

#define LOCTEXT_NAMESPACE "ActorIOEditor"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

SLATE_IMPLEMENT_WIDGET(SActorIOTab)
void SActorIOTab::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

void SActorIOTab::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SBox)
            .HeightOverride(30.0f)
            [
                SAssignNew(ActionPropertySplitter, SSplitter)
            ]
        ]
        + SVerticalBox::Slot()
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::Get().GetBrush("Brushes.Recessed"))
            [
                SAssignNew(ActionList, SVerticalBox)
            ]
        ]
	];

    InitializeHeaderRow();

    if (ActionPropertySizes.IsEmpty())
    {
        const int32 NumSlots = ActionPropertySplitter->GetChildren()->Num();
        ActionPropertySizes.Init(1.0f, NumSlots);

        for (int32 SlotIdx = 0; SlotIdx != NumSlots; ++SlotIdx)
        {
            const float TargetSize = ActionPropertySplitter->SlotAt(SlotIdx).GetSizeValue();
            ActionPropertySizes[SlotIdx] = TargetSize;
        }
    }

    for (int32 SlotIdx = 0; SlotIdx != ActionPropertySizes.Num(); ++SlotIdx)
    {
        OnActionPropertyResized(SlotIdx, ActionPropertySizes[SlotIdx]);
    }
}

void SActorIOTab::AddPropertyHeader(const FText& InPropertyName, const FMargin& InPadding)
{
    const int32 PropertyIndex = ActionPropertySplitter->GetChildren()->Num();

    ActionPropertySplitter->AddSlot()
    .OnSlotResized_Lambda([this, PropertyIndex](float InSize) { OnActionPropertyResized(PropertyIndex, InSize); })
    [
        SNew(SBox)
        .VAlign(VAlign_Center)
        .Padding(InPadding)
        [
            SNew(STextBlock)
            .Text(InPropertyName)
        ]
    ];
}

void SActorIOTab::OnActionPropertyResized(int32 InSlotIndex, float InSize)
{
    ActionPropertySizes[InSlotIndex] = InSize;
    ActionPropertySplitter->SlotAt(InSlotIndex).SetSizeValue(InSize);

    if (FChildren* ChildWidgets = ActionList->GetChildren())
    {
        ChildWidgets->ForEachWidget([InSlotIndex, InSize](SWidget& ChildWidget)
        {
            SActorIOAction& ActionWidget = static_cast<SActorIOAction&>(ChildWidget);
            ActionWidget.SetPropertySize(InSlotIndex, InSize);
        });
    }
}


SLATE_IMPLEMENT_WIDGET(SActorOutputsTab)
void SActorOutputsTab::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

void SActorOutputsTab::Construct(const FArguments& InArgs)
{
    Super::Construct(SActorIOTab::FArguments());
}

void SActorOutputsTab::InitializeHeaderRow()
{
    AddPropertyHeader(LOCTEXT("Event", "Event:"), FMargin(30.0f, 0.0f, 5.0f, 0.0f));
    AddPropertyHeader(LOCTEXT("Target", "Target:"), FMargin(5.0f, 0.0f));
    AddPropertyHeader(LOCTEXT("Action", "Action:"), FMargin(5.0f, 0.0f));
    AddPropertyHeader(LOCTEXT("Parameters", "Parameters:"), FMargin(5.0f, 0.0f));
}

void SActorOutputsTab::Refresh()
{
    ActionList->ClearChildren();

    UActorIOEditorSubsystem* ActorIOEditorSubsystem = GEditor->GetEditorSubsystem<UActorIOEditorSubsystem>();
    AActor* SelectedActor = ActorIOEditorSubsystem ? ActorIOEditorSubsystem->GetSelectedActor() : nullptr;
    UActorIOComponent* ActorIOComponent = SelectedActor ? SelectedActor->GetComponentByClass<UActorIOComponent>() : nullptr;

    if (ActorIOComponent)
    {
        for (UActorIOAction* Action : ActorIOComponent->GetActions())
        {
            ActionList->AddSlot()
            .AutoHeight()
            [
                SNew(SActorOutputAction)
                .Action(Action)
                .PropertySizes(ActionPropertySizes)
            ];
        }
    }
}


SLATE_IMPLEMENT_WIDGET(SActorInputsTab)
void SActorInputsTab::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

void SActorInputsTab::Construct(const FArguments& InArgs)
{
    Super::Construct(SActorIOTab::FArguments());
}

void SActorInputsTab::InitializeHeaderRow()
{
    AddPropertyHeader(LOCTEXT("Event", "Event:"), FMargin(30.0f, 0.0f, 5.0f, 0.0f));
    AddPropertyHeader(LOCTEXT("Target", "Target:"), FMargin(5.0f, 0.0f));
    AddPropertyHeader(LOCTEXT("Action", "Action:"), FMargin(5.0f, 0.0f));
    AddPropertyHeader(LOCTEXT("Parameters", "Parameters:"), FMargin(5.0f, 0.0f));
}

void SActorInputsTab::Refresh()
{
    ActionList->ClearChildren();

    UActorIOEditorSubsystem* ActorIOEditorSubsystem = GEditor->GetEditorSubsystem<UActorIOEditorSubsystem>();
    AActor* SelectedActor = ActorIOEditorSubsystem ? ActorIOEditorSubsystem->GetSelectedActor() : nullptr;

    TArray<TObjectPtr<UActorIOAction>> InputActions = UActorIOComponent::GetInputActionsForObject(SelectedActor);
    if (!InputActions.IsEmpty())
    {
        for (UActorIOAction* Action : InputActions)
        {
            ActionList->AddSlot()
            .AutoHeight()
            [
                SNew(SActorInputAction)
                .Action(Action)
                .PropertySizes(ActionPropertySizes)
            ];
        }
    }
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE

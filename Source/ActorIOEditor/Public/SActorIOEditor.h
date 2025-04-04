// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FReply;
enum class ECheckBoxState : uint8;

/**
 * Widget inside the I/O editor tab.
 */
class ACTORIOEDITOR_API SActorIOEditor : public SCompoundWidget
{
    SLATE_DECLARE_WIDGET(SActorIOEditor, SCompoundWidget)

public:

    SLATE_BEGIN_ARGS(SActorIOEditor)
    {}
    SLATE_END_ARGS()

    /** Widget constructor. */
    void Construct(const FArguments& InArgs);

    /**
     * Updates the widget to reflect the current state.
     * Used when the widget structure doesn't need to change, just the displayed values.
     */
    void Refresh();

    /** Set whether the editor should display input actions. If false, output actions are shown. */
    void SetViewInputActions(bool bEnabled, bool bRefresh = true);

protected:

    /** Image displaying the selected actor's icon. */
    TSharedPtr<class SImage> SelectedActorIcon;

    /** Text block displaying the selected actor's name. */
    TSharedPtr<class STextBlock> SelectedActorText;

    /** Text block of the view outputs button. */
    TSharedPtr<class STextBlock> OutputsButtonText;

    /** Text block of the view inputs button. */
    TSharedPtr<class STextBlock> InputsButtonText;

    /** Button to add new actions. */
    TSharedPtr<class SPositiveActionButton> NewActionButton;

    /** Border that the action list should be added to. */
    TSharedPtr<class SBorder> ActionListContainer;

    /** The action list that is displaying I/O actions. */
    TSharedPtr<class SActorIOActionListView> ActionListView;

    /** Whether the editor is displaying input actions. If false, output actions are shown. */
    bool bViewInputActions;

    /** Whether the action list should be refreshed with the next Refresh() call. */
    bool bActionListNeedsRegenerate;

protected:

    /** @return True if the view outputs button should be toggled. */
    ECheckBoxState IsOutputsButtonChecked() const;

    /** Called when the view outputs button is toggled. */
    void OnOutputsButtonChecked(ECheckBoxState InState);

    /** @return True if the view inputs button should be toggled. */
    ECheckBoxState IsInputsButtonChecked() const;

    /** Called when the view inputs button is toggled. */
    void OnInputsButtonChecked(ECheckBoxState InState);

    /** Called when the new action button is clicked. */
    FReply OnClick_NewAction();
};

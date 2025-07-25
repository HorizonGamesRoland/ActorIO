// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * Widget that displays all input parameters of a UFunction in the form of ParamName : ParamType.
 * Can highlight one parameter to indicate which one is currently being edited.
 */
class ACTORIOEDITOR_API SActorIOParamsViewer : public SCompoundWidget
{
	SLATE_DECLARE_WIDGET(SActorIOParamsViewer, SCompoundWidget)

public:

	SLATE_BEGIN_ARGS(SActorIOParamsViewer)
		: _FunctionPtr(nullptr)
	{}

	/** The UFunction to display the parameters of. */
	SLATE_ARGUMENT(UFunction*, FunctionPtr)
	/** Minimum width of the widget. */
	SLATE_ARGUMENT(float, MinDesiredWidth)

	SLATE_END_ARGS()

	/** Widget constructor. */
	void Construct(const FArguments& InArgs);

	/**
	 * Moves the highlight to given param.
	 * If INDEX_NONE, no param will be highlighted. 
	 */
	void SetHighlightedParam(int32 InParamIdx);

protected:

	/** List box containing the parameter entries. */
	TSharedPtr<class SVerticalBox> ParamsBox;

	/** Index of the currently highlighted param. */
	int32 HighlightedParamIdx;

protected:

	/** Generates the parameter entries from the given UFunction. */
	void FillParamsBox(UFunction* InFunctionPtr);

	/** @return Whether a param entry exists at the given index.  */
	bool IsValidParamIdx(int32 InParamIdx) const;

	/** @return The widget of the param entry at the given index. */
	TSharedRef<class SActorIOParamsViewerEntry> GetParamEntryAt(int32 InParamIdx) const;
};

/**
 * Widget for a single parameter entry in the SActorIOParamsViewer.
 */
class ACTORIOEDITOR_API SActorIOParamsViewerEntry : public SCompoundWidget
{
	SLATE_DECLARE_WIDGET(SActorIOParamsViewerEntry, SCompoundWidget)

public:

	SLATE_BEGIN_ARGS(SActorIOParamsViewerEntry)
		: _PropertyPtr(nullptr)
	{}

	/** The property of a UFunction to generate this entry from. */
	SLATE_ARGUMENT(FProperty*, PropertyPtr)

	SLATE_END_ARGS()

	/** Widget constructor. */
	void Construct(const FArguments& InArgs);

	/** Turns the widget highlight on/off. */
	void SetHighlightEnabled(bool bHighlight);

protected:

	/** Border that is used for the highlight. */
	TSharedPtr<class SBorder> WidgetBorder;
};

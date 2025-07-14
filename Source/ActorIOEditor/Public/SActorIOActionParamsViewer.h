// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class ACTORIOEDITOR_API SActorIOParamsViewer : public SCompoundWidget
{
	SLATE_DECLARE_WIDGET(SActorIOParamsViewer, SCompoundWidget)

public:

	SLATE_BEGIN_ARGS(SActorIOParamsViewer)
	{}

	/**  */
	SLATE_ARGUMENT(const UFunction*, FunctionPtr)

	SLATE_END_ARGS()

	/** Widget constructor. */
	void Construct(const FArguments& InArgs);

protected:

	TSharedPtr<class SWrapBox> ParamsBox;

protected:

	void FillParamsBox(const UFunction* InFunctionPtr);

	TSharedRef<SWidget> MakeParamWidgetForProperty(const FProperty* InProperty);
};

// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOWidgetIterator.h"
#include "Layout/ChildrenBase.h"

FActorIOChildWidgetIterator::FActorIOChildWidgetIterator(SWidget& InParent, TWidgetIterationFunc InIterationFunc)
{
	IterationFunc = InIterationFunc;
	Advance(InParent);
}

bool FActorIOChildWidgetIterator::Advance(SWidget& InWidget)
{
	if (!IterationFunc(InWidget))
	{
		return false;
	}

	FChildren* Children = InWidget.GetChildren();
	if (Children && Children->Num() > 0)
	{
		// Recursively go into child widgets.
		for (int32 ChildrenIdx = 0; ChildrenIdx != Children->Num(); ++ChildrenIdx)
		{
			if (!Advance(*Children->GetChildAt(ChildrenIdx)))
			{
				return false;
			}
		}
	}

	return true;
}

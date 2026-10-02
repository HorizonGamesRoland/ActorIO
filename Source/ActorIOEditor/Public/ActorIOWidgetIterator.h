// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "Widgets/SWidget.h"

/**
 * Class for iterating through all childs of a widget, including childrens of child widgets.
 * Iteration continues until the iterator func returns false, or we run out of widgets.
 */
class ACTORIOEDITOR_API FActorIOChildWidgetIterator
{
public:

    /**
     * Callback function when iterating over a widget.
     * Return false to stop the iterator.
     */
    typedef TFunction<bool(SWidget&)> TWidgetIterationFunc;

    /** Constructor. */
    FActorIOChildWidgetIterator(SWidget& InParent, TWidgetIterationFunc InIterationFunc);
    FActorIOChildWidgetIterator() = delete;

private:

    /** Recursively iterate over all child widgets of the given widget. */
    bool Advance(SWidget& InWidget);

    /** Function to call whenever we iterate over a widget. */
    TWidgetIterationFunc IterationFunc;
};